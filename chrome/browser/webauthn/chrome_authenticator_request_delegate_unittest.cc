// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/webauthn/chrome_authenticator_request_delegate.h"

#include <utility>

#include "build/build_config.h"
#include "chrome/test/base/chrome_render_view_host_test_harness.h"
#include "content/public/browser/authenticator_request_client_delegate.h"
#include "content/public/browser/browser_context.h"
#include "content/public/test/web_contents_tester.h"
#include "device/fido/fido_device_authenticator.h"
#include "device/fido/fido_discovery_factory.h"
#include "device/fido/test_callback_receiver.h"
#include "testing/gtest/include/gtest/gtest.h"

#if defined(OS_WIN)
#include "device/fido/win/authenticator.h"
#include "device/fido/win/fake_webauthn_api.h"
#include "third_party/microsoft_webauthn/webauthn.h"
#endif  // defined(OS_WIN)

#if defined(OS_MACOSX)
#include "device/fido/mac/authenticator_config.h"
#endif  // defined(OS_MACOSX)

class ChromeAuthenticatorRequestDelegateTest
    : public ChromeRenderViewHostTestHarness {};

TEST_F(ChromeAuthenticatorRequestDelegateTest, TestTransportPrefType) {
  ChromeAuthenticatorRequestDelegate delegate(main_rfh());
  EXPECT_FALSE(delegate.GetLastTransportUsed());
  delegate.UpdateLastTransportUsed(device::FidoTransportProtocol::kInternal);
  const auto transport = delegate.GetLastTransportUsed();
  ASSERT_TRUE(transport);
  EXPECT_EQ(device::FidoTransportProtocol::kInternal, transport);
}

TEST_F(ChromeAuthenticatorRequestDelegateTest,
       TestPairedDeviceAddressPreference) {
  static constexpr char kTestPairedDeviceAddress[] = "paired_device_address";
  static constexpr char kTestPairedDeviceAddress2[] = "paired_device_address2";

  ChromeAuthenticatorRequestDelegate delegate(main_rfh());

  auto* const address_list = delegate.GetPreviouslyPairedFidoBleDeviceIds();
  ASSERT_TRUE(address_list);
  EXPECT_TRUE(address_list->empty());

  delegate.AddFidoBleDeviceToPairedList(kTestPairedDeviceAddress);
  const auto* updated_address_list =
      delegate.GetPreviouslyPairedFidoBleDeviceIds();
  ASSERT_TRUE(updated_address_list);
  ASSERT_EQ(1u, updated_address_list->GetSize());

  const auto& address_value = updated_address_list->GetList()[0];
  ASSERT_TRUE(address_value.is_string());
  EXPECT_EQ(kTestPairedDeviceAddress, address_value.GetString());

  delegate.AddFidoBleDeviceToPairedList(kTestPairedDeviceAddress);
  const auto* address_list_with_duplicate_address_added =
      delegate.GetPreviouslyPairedFidoBleDeviceIds();
  ASSERT_TRUE(address_list_with_duplicate_address_added);
  EXPECT_EQ(1u, address_list_with_duplicate_address_added->GetSize());

  delegate.AddFidoBleDeviceToPairedList(kTestPairedDeviceAddress2);
  const auto* address_list_with_two_addresses =
      delegate.GetPreviouslyPairedFidoBleDeviceIds();
  ASSERT_TRUE(address_list_with_two_addresses);

  ASSERT_EQ(2u, address_list_with_two_addresses->GetSize());
  const auto& second_address_value =
      address_list_with_two_addresses->GetList()[1];
  ASSERT_TRUE(second_address_value.is_string());
  EXPECT_EQ(kTestPairedDeviceAddress2, second_address_value.GetString());
}

#if defined(OS_MACOSX)
API_AVAILABLE(macos(10.12.2))
std::string TouchIdMetadataSecret(
    ChromeAuthenticatorRequestDelegate* delegate) {
  return delegate->GetTouchIdAuthenticatorConfig()->metadata_secret;
}

TEST_F(ChromeAuthenticatorRequestDelegateTest, TouchIdMetadataSecret) {
  if (__builtin_available(macOS 10.12.2, *)) {
    ChromeAuthenticatorRequestDelegate delegate(main_rfh());
    std::string secret = TouchIdMetadataSecret(&delegate);
    EXPECT_EQ(secret.size(), 32u);
    EXPECT_EQ(secret, TouchIdMetadataSecret(&delegate));
  }
}

TEST_F(ChromeAuthenticatorRequestDelegateTest,
       TouchIdMetadataSecret_EqualForSameProfile) {
  if (__builtin_available(macOS 10.12.2, *)) {
    // Different delegates on the same BrowserContext (Profile) should return
    // the same secret.
    ChromeAuthenticatorRequestDelegate delegate1(main_rfh());
    ChromeAuthenticatorRequestDelegate delegate2(main_rfh());
    EXPECT_EQ(TouchIdMetadataSecret(&delegate1),
              TouchIdMetadataSecret(&delegate2));
  }
}

TEST_F(ChromeAuthenticatorRequestDelegateTest,
       TouchIdMetadataSecret_NotEqualForDifferentProfiles) {
  if (__builtin_available(macOS 10.12.2, *)) {
    // Different profiles have different secrets. (No way to reset
    // browser_context(), so we have to create our own.)
    auto browser_context = CreateBrowserContext();
    auto web_contents = content::WebContentsTester::CreateTestWebContents(
        browser_context.get(), nullptr);
    ChromeAuthenticatorRequestDelegate delegate1(main_rfh());
    ChromeAuthenticatorRequestDelegate delegate2(web_contents->GetMainFrame());
    EXPECT_NE(TouchIdMetadataSecret(&delegate1),
              TouchIdMetadataSecret(&delegate2));
    // Ensure this second secret is actually valid.
    EXPECT_EQ(32u, TouchIdMetadataSecret(&delegate2).size());
  }
}
#endif  // defined(OS_MACOSX)

#if defined(OS_WIN)

static constexpr char kRelyingPartyID[] = "example.com";

// Tests that ShouldReturnAttestation() returns with true if |authenticator|
// is the Windows native WebAuthn API with WEBAUTHN_API_VERSION_2 or higher,
// where Windows prompts for attestation in its own native UI.
//
// Ideally, this would also test the inverse case, i.e. that with
// WEBAUTHN_API_VERSION_1 Chrome's own attestation prompt is shown. However,
// there seems to be no good way to test AuthenticatorRequestDialogModel UI.
TEST_F(ChromeAuthenticatorRequestDelegateTest, ShouldPromptForAttestationWin) {
  ::device::FakeWinWebAuthnApi win_webauthn_api;
  win_webauthn_api.set_version(WEBAUTHN_API_VERSION_2);
  ::device::WinWebAuthnApiAuthenticator authenticator(
      /*current_window=*/nullptr, &win_webauthn_api);

  ::device::test::ValueCallbackReceiver<bool> cb;
  ChromeAuthenticatorRequestDelegate delegate(main_rfh());
  delegate.ShouldReturnAttestation(kRelyingPartyID, &authenticator,
                                   cb.callback());
  cb.WaitForCallback();
  EXPECT_EQ(cb.value(), true);
}

#endif  // defined(OS_WIN)
