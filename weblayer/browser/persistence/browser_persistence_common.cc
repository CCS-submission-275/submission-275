// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "weblayer/browser/persistence/browser_persistence_common.h"

#include "components/sessions/content/content_serialized_navigation_builder.h"
#include "components/sessions/content/session_tab_helper.h"
#include "components/sessions/core/session_command.h"
#include "components/sessions/core/session_service_commands.h"
#include "components/sessions/core/session_types.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_url_handler.h"
#include "content/public/browser/dom_storage_context.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/storage_partition.h"
#include "weblayer/browser/browser_impl.h"
#include "weblayer/browser/profile_impl.h"
#include "weblayer/browser/tab_impl.h"

namespace weblayer {
namespace {

void ProcessRestoreCommands(
    BrowserImpl* browser,
    const std::vector<std::unique_ptr<sessions::SessionWindow>>& windows) {
  if (windows.empty() || windows[0]->tabs.empty())
    return;

  const bool had_tabs = !browser->GetTabs().empty();
  content::BrowserContext* browser_context =
      browser->profile()->GetBrowserContext();
  for (int i = 0; i < static_cast<int>(windows[0]->tabs.size()); ++i) {
    const sessions::SessionTab& session_tab = *(windows[0]->tabs[i]);
    if (session_tab.navigations.empty())
      continue;

    // Associate sessionStorage (if any) to the restored tab.
    scoped_refptr<content::SessionStorageNamespace> session_storage_namespace;
    if (!session_tab.session_storage_persistent_id.empty()) {
      session_storage_namespace =
          content::BrowserContext::GetDefaultStoragePartition(browser_context)
              ->GetDOMStorageContext()
              ->RecreateSessionStorage(
                  session_tab.session_storage_persistent_id);
    }

    const int selected_navigation_index =
        session_tab.normalized_navigation_index();

    GURL restore_url =
        session_tab.navigations[selected_navigation_index].virtual_url();
    content::SessionStorageNamespaceMap session_storage_namespace_map;
    session_storage_namespace_map[std::string()] = session_storage_namespace;
    content::BrowserURLHandler::GetInstance()->RewriteURLIfNecessary(
        &restore_url, browser_context);
    content::WebContents::CreateParams create_params(
        browser_context,
        content::SiteInstance::ShouldAssignSiteForURL(restore_url)
            ? content::SiteInstance::CreateForURL(browser_context, restore_url)
            : nullptr);
    create_params.initially_hidden = true;
    create_params.desired_renderer_state =
        content::WebContents::CreateParams::kNoRendererProcess;
    create_params.last_active_time = session_tab.last_active_time;
    std::unique_ptr<content::WebContents> web_contents =
        content::WebContents::CreateWithSessionStorage(
            create_params, session_storage_namespace_map);
    std::vector<std::unique_ptr<content::NavigationEntry>> entries =
        sessions::ContentSerializedNavigationBuilder::ToNavigationEntries(
            session_tab.navigations, browser_context);
    // TODO(https://crbug.com/1061917): handle UA client hints override.
    web_contents->SetUserAgentOverride(blink::UserAgentOverride::UserAgentOnly(
                                           session_tab.user_agent_override),
                                       false);
    // CURRENT_SESSION matches what clank does. On the desktop, we should
    // use a different type.
    web_contents->GetController().Restore(selected_navigation_index,
                                          content::RestoreType::CURRENT_SESSION,
                                          &entries);
    DCHECK(entries.empty());
    TabImpl* tab = browser->CreateTabForSessionRestore(std::move(web_contents),
                                                       session_tab.guid);

    if (!had_tabs && i == (windows[0])->selected_tab_index)
      browser->SetActiveTab(tab);
  }
  if (!had_tabs && !browser->GetTabs().empty() && !browser->GetActiveTab())
    browser->SetActiveTab(browser->GetTabs().back());
}

}  // namespace

void RestoreBrowserState(
    BrowserImpl* browser,
    std::vector<std::unique_ptr<sessions::SessionCommand>> commands) {
  std::vector<std::unique_ptr<sessions::SessionWindow>> windows;
  SessionID active_window_id = SessionID::InvalidValue();
  sessions::RestoreSessionFromCommands(commands, &windows, &active_window_id);
  ProcessRestoreCommands(browser, windows);

  if (browser->GetTabs().empty()) {
    // Nothing to restore, or restore failed. Create a default tab.
    browser->SetActiveTab(
        browser->CreateTabForSessionRestore(nullptr, std::string()));
  }
}

std::vector<std::unique_ptr<sessions::SessionCommand>>
BuildCommandsForTabConfiguration(const SessionID& browser_session_id,
                                 TabImpl* tab,
                                 int index_in_browser) {
  DCHECK(tab);
  std::vector<std::unique_ptr<sessions::SessionCommand>> result;
  const SessionID& tab_id = GetSessionIDForTab(tab);
  result.push_back(
      sessions::CreateSetTabWindowCommand(browser_session_id, tab_id));

  result.push_back(sessions::CreateLastActiveTimeCommand(
      tab_id, tab->web_contents()->GetLastActiveTime()));

  // TODO(https://crbug.com/1061917): handle UA client hints override.
  const std::string& ua_override =
      tab->web_contents()->GetUserAgentOverride().ua_string_override;
  if (!ua_override.empty()) {
    result.push_back(
        sessions::CreateSetTabUserAgentOverrideCommand(tab_id, ua_override));
  }
  if (index_in_browser != -1) {
    result.push_back(
        sessions::CreateSetTabIndexInWindowCommand(tab_id, index_in_browser));
  }

  result.push_back(sessions::CreateSetSelectedNavigationIndexCommand(
      tab_id, tab->web_contents()->GetController().GetCurrentEntryIndex()));

  result.push_back(sessions::CreateSetTabGuidCommand(tab_id, tab->GetGuid()));

  return result;
}

const SessionID& GetSessionIDForTab(Tab* tab) {
  sessions::SessionTabHelper* session_tab_helper =
      sessions::SessionTabHelper::FromWebContents(
          static_cast<TabImpl*>(tab)->web_contents());
  DCHECK(session_tab_helper);
  return session_tab_helper->session_id();
}

}  // namespace weblayer
