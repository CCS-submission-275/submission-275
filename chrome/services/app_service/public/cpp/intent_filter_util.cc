// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/services/app_service/public/cpp/intent_filter_util.h"

namespace {

bool ConditionsHaveOverlap(const apps::mojom::ConditionPtr& condition1,
                           const apps::mojom::ConditionPtr& condition2) {
  if (condition1->condition_type != condition2->condition_type) {
    return false;
  }
  // If there are same |condition_value| exist in the both |condition|s, there
  // is an overlap.
  for (auto& value1 : condition1->condition_values) {
    for (auto& value2 : condition2->condition_values) {
      if (value1 == value2) {
        return true;
      }
    }
  }
  return false;
}

}  // namespace

namespace apps_util {

apps::mojom::ConditionValuePtr MakeConditionValue(
    const std::string& value,
    apps::mojom::PatternMatchType pattern_match_type) {
  auto condition_value = apps::mojom::ConditionValue::New();
  condition_value->value = value;
  condition_value->match_type = pattern_match_type;

  return condition_value;
}

apps::mojom::ConditionPtr MakeCondition(
    apps::mojom::ConditionType condition_type,
    std::vector<apps::mojom::ConditionValuePtr> condition_values) {
  auto condition = apps::mojom::Condition::New();
  condition->condition_type = condition_type;
  condition->condition_values = std::move(condition_values);

  return condition;
}

apps::mojom::IntentFilterPtr CreateIntentFilterForUrlScope(const GURL& url) {
  auto intent_filter = apps::mojom::IntentFilter::New();

  std::vector<apps::mojom::ConditionValuePtr> scheme_condition_values;
  scheme_condition_values.push_back(apps_util::MakeConditionValue(
      url.scheme(), apps::mojom::PatternMatchType::kNone));
  auto scheme_condition = apps_util::MakeCondition(
      apps::mojom::ConditionType::kScheme, std::move(scheme_condition_values));
  intent_filter->conditions.push_back(std::move(scheme_condition));

  std::vector<apps::mojom::ConditionValuePtr> host_condition_values;
  host_condition_values.push_back(apps_util::MakeConditionValue(
      url.host(), apps::mojom::PatternMatchType::kNone));
  auto host_condition = apps_util::MakeCondition(
      apps::mojom::ConditionType::kHost, std::move(host_condition_values));
  intent_filter->conditions.push_back(std::move(host_condition));

  std::vector<apps::mojom::ConditionValuePtr> path_condition_values;
  path_condition_values.push_back(apps_util::MakeConditionValue(
      url.path(), apps::mojom::PatternMatchType::kPrefix));
  auto path_condition = apps_util::MakeCondition(
      apps::mojom::ConditionType::kPattern, std::move(path_condition_values));
  intent_filter->conditions.push_back(std::move(path_condition));

  return intent_filter;
}

int GetFilterMatchLevel(const apps::mojom::IntentFilterPtr& intent_filter) {
  int match_level = IntentFilterMatchLevel::kNone;
  for (const auto& condition : intent_filter->conditions) {
    switch (condition->condition_type) {
      case apps::mojom::ConditionType::kScheme:
        match_level += IntentFilterMatchLevel::kScheme;
        break;
      case apps::mojom::ConditionType::kHost:
        match_level += IntentFilterMatchLevel::kHost;
        break;
      case apps::mojom::ConditionType::kPattern:
        match_level += IntentFilterMatchLevel::kPattern;
        break;
    }
  }
  return match_level;
}

bool FiltersHaveOverlap(const apps::mojom::IntentFilterPtr& filter1,
                        const apps::mojom::IntentFilterPtr& filter2) {
  if (GetFilterMatchLevel(filter1) != GetFilterMatchLevel(filter2)) {
    return false;
  }
  for (size_t i = 0; i < filter1->conditions.size(); i++) {
    auto& condition1 = filter1->conditions[i];
    auto& condition2 = filter2->conditions[i];
    if (!ConditionsHaveOverlap(condition1, condition2)) {
      return false;
    }
  }
  return true;
}

}  // namespace apps_util
