// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/public/cpp/app_menu_constants.h"
#include "ash/public/cpp/shelf_item_delegate.h"
#include "ash/public/cpp/shelf_model.h"
#include "base/bind_helpers.h"
#include "base/run_loop.h"
#include "base/test/bind_test_util.h"
#include "chrome/browser/apps/app_service/app_service_proxy.h"
#include "chrome/browser/apps/app_service/app_service_proxy_factory.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/web_applications/test/web_app_browsertest_util.h"
#include "chrome/browser/web_applications/components/web_app_id.h"
#include "chrome/browser/web_applications/test/web_app_test.h"
#include "chrome/common/chrome_features.h"
#include "chrome/common/web_application_info.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "ui/base/models/simple_menu_model.h"
#include "ui/display/display.h"

using web_app::ProviderType;

class AppServiceShelfContextMenuWebAppBrowserTest
    : public InProcessBrowserTest,
      public ::testing::WithParamInterface<ProviderType> {
 public:
  AppServiceShelfContextMenuWebAppBrowserTest() = default;
  ~AppServiceShelfContextMenuWebAppBrowserTest() override = default;

  void SetUp() override {
    if (GetParam() == ProviderType::kWebApps) {
      scoped_feature_list_.InitWithFeatures(
          {features::kDesktopPWAsTabStrip,
           features::kDesktopPWAsWithoutExtensions},
          {});
    } else {
      DCHECK_EQ(GetParam(), ProviderType::kBookmarkApps);
      scoped_feature_list_.InitWithFeatures(
          {features::kDesktopPWAsTabStrip},
          {features::kDesktopPWAsWithoutExtensions});
    }

    InProcessBrowserTest::SetUp();
  }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

IN_PROC_BROWSER_TEST_P(AppServiceShelfContextMenuWebAppBrowserTest,
                       SetOpenInTabbedWindow) {
  Profile* profile = browser()->profile();

  auto web_application_info = std::make_unique<WebApplicationInfo>();
  web_application_info->app_url = GURL("https://example.org");
  web_app::AppId app_id =
      web_app::InstallWebApp(profile, std::move(web_application_info));

  // Wait for app service to see the newly installed app.
  apps::AppServiceProxyFactory::GetForProfile(profile)
      ->FlushMojoCallsForTesting();

  // Open app shelf icon context menu.
  ash::ShelfModel* shelf_model = ash::ShelfModel::Get();
  shelf_model->PinAppWithID(app_id);
  ash::ShelfItemDelegate* delegate =
      shelf_model->GetShelfItemDelegate(ash::ShelfID(app_id));
  base::RunLoop run_loop;
  std::unique_ptr<ui::SimpleMenuModel> menu_model;
  delegate->GetContextMenu(
      display::Display::GetDefaultDisplay().id(),
      base::BindLambdaForTesting(
          [&run_loop, &menu_model](std::unique_ptr<ui::SimpleMenuModel> model) {
            menu_model = std::move(model);
            run_loop.Quit();
          }));
  run_loop.Run();

  // Set app to open in tabbed window.
  ui::MenuModel* sub_model = menu_model.get();
  int command_index = -1;
  ASSERT_TRUE(ui::MenuModel::GetModelAndIndexForCommandId(
      ash::LAUNCH_TYPE_TABBED_WINDOW, &sub_model, &command_index));
  sub_model->ActivatedAt(command_index);

  Browser* app_browser = web_app::LaunchWebAppBrowser(profile, app_id);
  EXPECT_TRUE(app_browser->app_controller()->has_tab_strip());
}

INSTANTIATE_TEST_SUITE_P(All,
                         AppServiceShelfContextMenuWebAppBrowserTest,
                         ::testing::Values(ProviderType::kBookmarkApps,
                                           ProviderType::kWebApps),
                         web_app::ProviderTypeParamToString);
