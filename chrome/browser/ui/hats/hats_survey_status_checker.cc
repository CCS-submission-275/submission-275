// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/hats/hats_survey_status_checker.h"

#include "base/strings/string_util.h"
#include "base/timer/timer.h"
#include "content/public/browser/storage_partition.h"
#include "net/base/load_flags.h"
#include "net/cookies/canonical_cookie.h"
#include "net/http/http_status_code.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"
#include "url/gurl.h"

constexpr char HatsSurveyStatusChecker::kHatsSurveyDataPath[];
constexpr char HatsSurveyStatusChecker::kReasonHeader[];
constexpr char HatsSurveyStatusChecker::kReasonOverCapacity[];

HatsSurveyStatusChecker::HatsSurveyStatusChecker(Profile* profile)
    : otr_profile_registration_(
          IndependentOTRProfileManager::GetInstance()
              ->CreateFromOriginalProfile(
                  profile,
                  base::BindOnce(
                      &HatsSurveyStatusChecker::OnOriginalProfileDestroyed,
                      base::Unretained(this)))) {
  DCHECK(
      otr_profile_registration_->profile()->IsIndependentOffTheRecordProfile());

  // HaTS client first downloads a javascript library from
  // https://www.google.com/insights/consumersurveys/async_survey?site=<site_id>.
  // HaTS server then responds and sets a site cookie "PAIDCONTENT=<id>".
  // This cookie will be sent along with the subsequent survey fetching request
  // as privacy information. On the server side, the presence of this cookie
  // will be checked. If the check fails, an error returns stating that privacy
  // info doesn't meet requirements. However, the cookie's value is not checked.
  // To fetch the survey status directly, we need to set the cookie, but it is
  // ok to use a non-conforming value.
  auto survey_cookie = net::CanonicalCookie::Create(
      GURL("https://www.google.com"), "PAIDCONTENT=0", base::Time::Now(),
      base::nullopt);
  content::StoragePartition* partition =
      content::BrowserContext::GetDefaultStoragePartition(
          otr_profile_registration_->profile());
  network::mojom::CookieManager* cookie_manager =
      partition->GetCookieManagerForBrowserProcess();
  cookie_manager->SetCanonicalCookie(*survey_cookie, "https",
                                     net::CookieOptions::MakeAllInclusive(),
                                     base::DoNothing());
}

HatsSurveyStatusChecker::~HatsSurveyStatusChecker() = default;

void HatsSurveyStatusChecker::CheckSurveyStatus(
    const std::string& site_id,
    base::OnceClosure on_success,
    base::OnceCallback<void(Status)> on_failure) {
  if (url_loader_) {
    // There is a pending survey check, no need to repeat the effort.
    return;
  }

  on_success_ = std::move(on_success);
  on_failure_ = std::move(on_failure);

  // Send the request and check the response header.
  auto request = std::make_unique<network::ResourceRequest>();
  std::string url_without_id(HatsSurveyURLWithoutId());
  request->url = GURL(url_without_id + site_id);
  // Send stored cookie along with the request, but don't save any cookie.
  request->attach_same_site_cookies = true;
  request->load_flags = net::LOAD_BYPASS_CACHE | net::LOAD_DISABLE_CACHE |
                        net::LOAD_DO_NOT_SAVE_COOKIES;
  DCHECK(!url_loader_);
  url_loader_ = network::SimpleURLLoader::Create(
      std::move(request),
      net::DefineNetworkTrafficAnnotation("chrome_HaTS_service", R"(
        semantics {
          sender: "Chrome HaTS Service"
          description:
            "Checks if the HaTS server can accept new surveys."
          trigger:
            "The HaTS service makes a request once per Chrome run right before "
            "showing the HaTS bubble."
          data:
            "A cookie with a fake value is sent since the cookie's presence is "
            "required. No user data is sent."
          destination: GOOGLE_OWNED_SERVICE
        }
        policy {
          cookies_allowed: YES
          cookies_store: "An ephemeral cookie jar used for one request."
          setting:
            "This feature cannot be disabled in settings."
          policy_exception_justification:
            "Not implemented."
        })"));
  content::StoragePartition* partition =
      content::BrowserContext::GetDefaultStoragePartition(
          otr_profile_registration_->profile());
  url_loader_->DownloadHeadersOnly(
      partition->GetURLLoaderFactoryForBrowserProcess().get(),
      base::BindOnce(&HatsSurveyStatusChecker::OnURLLoadComplete,
                     base::Unretained(this)));
  request_timer_.Start(FROM_HERE,
                       base::TimeDelta::FromSeconds(SurveyCheckTimeoutSecs()),
                       base::BindOnce(&HatsSurveyStatusChecker::OnTimeout,
                                      base::Unretained(this)));
}

base::OnceClosure HatsSurveyStatusChecker::CreateTimeoutCallbackForTesting() {
  return base::BindOnce(&HatsSurveyStatusChecker::OnTimeout,
                        base::Unretained(this));
}

HatsSurveyStatusChecker::HatsSurveyStatusChecker() = default;

std::string HatsSurveyStatusChecker::HatsSurveyURLWithoutId() {
  std::string url("https://www.google.com/");
  return url + kHatsSurveyDataPath;
}

int HatsSurveyStatusChecker::SurveyCheckTimeoutSecs() {
  return kTimeoutSecs;
}

void HatsSurveyStatusChecker::OnOriginalProfileDestroyed(Profile* profile) {
  if (otr_profile_registration_ &&
      profile == otr_profile_registration_->profile()) {
    otr_profile_registration_.reset();
  }
}

void HatsSurveyStatusChecker::OnURLLoadComplete(
    scoped_refptr<net::HttpResponseHeaders> headers) {
  if (!url_loader_) {
    // If there is no pending URL request, OnTimeout() has already run.
    return;
  }

  std::unique_ptr<network::SimpleURLLoader> url_loader = std::move(url_loader_);

  if (!headers || headers->response_code() != net::HTTP_OK) {
    std::move(on_failure_).Run(Status::kResponseHeaderError);
    return;
  }

  std::string header_value;
  if (headers->GetNormalizedHeader(HatsSurveyStatusChecker::kReasonHeader,
                                   &header_value) &&
      base::StartsWith(header_value,
                       HatsSurveyStatusChecker::kReasonOverCapacity,
                       base::CompareCase::INSENSITIVE_ASCII)) {
    std::move(on_failure_).Run(Status::kOverCapacity);
    return;
  }
  std::move(on_success_).Run();
}

void HatsSurveyStatusChecker::OnTimeout() {
  if (!url_loader_) {
    // If there is no pending URL request, OnURLLoadComplete() has already run.
    return;
  }

  std::unique_ptr<network::SimpleURLLoader> url_loader = std::move(url_loader_);
  std::move(on_failure_).Run(Status::kUnreachable);
}
