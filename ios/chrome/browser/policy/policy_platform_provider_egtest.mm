// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/testing/earl_grey/earl_grey_test.h"

#include <memory>

#include "base/json/json_string_value_serializer.h"
#include "base/strings/sys_string_conversions.h"
#import "components/policy/core/common/policy_loader_ios_constants.h"
#include "ios/chrome/browser/chrome_switches.h"
#import "ios/chrome/browser/policy/policy_app_interface.h"
#include "ios/chrome/browser/pref_names.h"
#include "ios/chrome/test/earl_grey/chrome_earl_grey.h"
#include "ios/chrome/test/earl_grey/chrome_test_case.h"
#include "ios/testing/earl_grey/app_launch_configuration.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {

// Returns the value of a given policy, looked up in the current platform policy
// provider.
std::unique_ptr<base::Value> GetPlatformPolicy(const std::string& key) {
  std::string json_representation = base::SysNSStringToUTF8(
      [PolicyAppInterface valueForPlatformPolicy:base::SysUTF8ToNSString(key)]);
  JSONStringValueDeserializer deserializer(json_representation);
  return deserializer.Deserialize(/*error_code=*/nullptr,
                                  /*error_message=*/nullptr);
}

// Returns an AppLaunchConfiguration containing the given policy data.
// |policyData| must be in XML format.
AppLaunchConfiguration GenerateAppLaunchConfiguration(std::string policy_data) {
  AppLaunchConfiguration config;
  config.additional_args.push_back(std::string("--") +
                                   switches::kEnableEnterprisePolicy);

  // Remove whitespace from the policy data, because the XML parser does not
  // tolerate newlines.
  base::RemoveChars(policy_data, base::kWhitespaceASCII, &policy_data);

  // Commandline flags that start with a single "-" are automatically added to
  // the NSArgumentDomain in NSUserDefaults. Set fake policy data that can be
  // read by the production platform policy provider.
  config.additional_args.push_back(
      "-" + base::SysNSStringToUTF8(kPolicyLoaderIOSConfigurationKey));
  config.additional_args.push_back(policy_data);

  config.relaunch_policy = NoForceRelaunchAndResetState;
  return config;
}

}  // namespace

// Test case that uses the production platform policy provider.
@interface PolicyPlatformProviderTestCase : ChromeTestCase
@end

@implementation PolicyPlatformProviderTestCase

- (AppLaunchConfiguration)appConfigurationForTestCase {
  const std::string loadPolicyKey =
      base::SysNSStringToUTF8(kPolicyLoaderIOSLoadPolicyKey);
  std::string policyData = "<dict>"
                           "    <key>" +
                           loadPolicyKey +
                           "</key>"
                           "    <true/>"
                           "    <key>DefaultSearchProviderName</key>"
                           "    <string>Test</string>"
                           "    <key>NotARegisteredPolicy</key>"
                           "    <string>Unknown</string>"
                           "    <key>SearchSuggestEnabled</key>"
                           "    <false/>"
                           "</dict>";
  return GenerateAppLaunchConfiguration(policyData);
}

// Tests the values of policies that were explicitly set.
- (void)testPolicyExplicitlySet {
  std::unique_ptr<base::Value> searchValue =
      GetPlatformPolicy("DefaultSearchProviderName");
  GREYAssertTrue(searchValue && searchValue->is_string(),
                 @"searchValue was not of type string");
  GREYAssertEqual(searchValue->GetString(), "Test",
                  @"searchValue had an unexpected value");

  std::unique_ptr<base::Value> suggestValue =
      GetPlatformPolicy("SearchSuggestEnabled");
  GREYAssertTrue(suggestValue && suggestValue->is_bool(),
                 @"suggestValue was not of type bool");
  GREYAssertFalse(suggestValue->GetBool(),
                  @"suggestValue had an unexpected value");
}

// Test the value of a policy that exists in the schema but was not explicitly
// set.
- (void)testPolicyNotSet {
  std::unique_ptr<base::Value> blocklistValue =
      GetPlatformPolicy("URLBlacklist");
  GREYAssertTrue(blocklistValue && blocklistValue->is_none(),
                 @"blocklistValue was unexpectedly present");
}

// Test the value of a policy that was set in the configuration but is unknown
// to the policy system.
- (void)testPolicyUnknown {
  std::unique_ptr<base::Value> unknownValue =
      GetPlatformPolicy("NotARegisteredPolicy");
  GREYAssertTrue(unknownValue && unknownValue->is_string(),
                 @"unknownValue was not of type string");
  GREYAssertEqual(unknownValue->GetString(), "Unknown",
                  @"unknownValue had an unexpected value");
}

// Verifies that the kPolicyLoaderIOSLoadPolicyKey field is not loaded, even
// though it is present in the policy data.
- (void)testLoadPolicyKeyIsStripped {
  std::unique_ptr<base::Value> loadValue =
      GetPlatformPolicy(base::SysNSStringToUTF8(kPolicyLoaderIOSLoadPolicyKey));
  GREYAssertTrue(loadValue && loadValue->is_none(),
                 @"The LoadPolicy key was unexpectedly present in policy data");
}

@end

// Test case that uses the production platform policy provider but does not pass
// the special key that enables policy support.
@interface PolicyNotEnabledTestCase : ChromeTestCase
@end

@implementation PolicyNotEnabledTestCase

- (AppLaunchConfiguration)appConfigurationForTestCase {
  std::string policyData = "<dict>"
                           "    <key>DefaultSearchProviderName</key>"
                           "    <string>Test</string>"
                           "</dict>";
  return GenerateAppLaunchConfiguration(policyData);
}

// Tests that policies are not loaded unless kPolicyLoaderIOSLoadPolicyKey is
// set.
- (void)testLoadPolicyKeyNotSet {
  std::unique_ptr<base::Value> searchValue =
      GetPlatformPolicy("DefaultSearchProviderName");
  GREYAssertTrue(searchValue && searchValue->is_none(),
                 @"searchValue was unexpectedly present");
}

@end
