// SPDX-License-Identifier: Apache-2.0
//
// Copyright 2026 JASPAR
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "grpc/viss_grpc_subscribe_util.h"

#include <cstdint>

#include "yoriito/viss/v1/types/filter.pb.h"
//
#include "databroker/filter/change_filter.h"
#include "databroker/filter/range_condition.h"
#include "databroker/filter/subscribe_filter.h"

namespace yoriito::databroker {

void VissGrpcSubscribeUtil::SetFilter(
    yoriito::viss::v1::SubscribeFilter& filter,
    const yoriito::databroker::filter::SubscribeFilter& data_filter) {
  const auto& paths_filter = data_filter.GetPathsFilter();
  const auto& misc_filter = data_filter.GetMiscFilter();
  // pattern1 PathsFilter Only
  if (paths_filter && std::holds_alternative<std::monostate>(misc_filter)) {
    auto paths_filter_proto = filter.mutable_paths_parameter();
    auto paths = paths_filter.value().GetPaths();
    for (const std::string& path : paths) {
      paths_filter_proto->add_paths(path);
    }
  } else if (paths_filter &&
             !std::holds_alternative<std::monostate>(misc_filter)) {
    // pattern2 ComboFilter(Paths + Misc  )
    auto combo_filter = filter.mutable_combo_filter();
    auto paths_filter_proto = combo_filter->mutable_paths_parameter();
    auto paths = paths_filter.value().GetPaths();
    for (const std::string& path : paths) {
      paths_filter_proto->add_paths(path);
    }

  } else {
    // Pattern3 MiscFilter(Range , TimedBase , Change)
    auto viss_misc_filter = filter.mutable_misc_parameter();
    if (std::holds_alternative<yoriito::databroker::filter::ChangeFilter>(
            data_filter.GetMiscFilter())) {
      //
      auto change_filter = std::get<yoriito::databroker::filter::ChangeFilter>(
          data_filter.GetMiscFilter());
      auto change = viss_misc_filter->mutable_change_parameter();
      auto logic_operator = change_filter.GetLogicOperator();
      switch (logic_operator) {
        case yoriito::databroker::filter::OperatorType::OPERATOR_UNDEFINED:
          change->set_logic_op(yoriito::viss::v1::Operator::OPERATOR_UNDEFINED);
          break;
        case yoriito::databroker::filter::OperatorType::OPERATOR_EQ:
          change->set_logic_op(yoriito::viss::v1::Operator::OPERATOR_EQ);
          break;
        case yoriito::databroker::filter::OperatorType::OPERATOR_NE:
          change->set_logic_op(yoriito::viss::v1::Operator::OPERATOR_NE);
          break;
        case yoriito::databroker::filter::OperatorType::OPERATOR_GT:
          change->set_logic_op(yoriito::viss::v1::Operator::OPERATOR_GT);
          break;
        case yoriito::databroker::filter::OperatorType::OPERATOR_GTE:
          change->set_logic_op(yoriito::viss::v1::Operator::OPERATOR_GTE);
          break;
        case yoriito::databroker::filter::OperatorType::OPERATOR_LT:
          change->set_logic_op(yoriito::viss::v1::Operator::OPERATOR_LT);
          break;
        case yoriito::databroker::filter::OperatorType::OPERATOR_LTE:
          change->set_logic_op(yoriito::viss::v1::Operator::OPERATOR_LTE);
          break;
        default:
          break;
      }
      change->set_diff(change_filter.GetDiff());
    } else if (std::holds_alternative<yoriito::databroker::filter::RangeFilter>(
                   data_filter.GetMiscFilter())) {
      //
      auto& range_filter = std::get<yoriito::databroker::filter::RangeFilter>(
          data_filter.GetMiscFilter());
      auto range = viss_misc_filter->mutable_range_parameter();
      if (range_filter.IsSingleCondition()) {
        auto single_condition = range_filter.GetSingleCondition();
        auto single = range->mutable_single();
        switch (single_condition->GetBoundaryOperator()) {
          case yoriito::databroker::filter::OperatorType::OPERATOR_UNDEFINED:
            single->set_boundary_op(
                yoriito::viss::v1::Operator::OPERATOR_UNDEFINED);
            break;
          case yoriito::databroker::filter::OperatorType::OPERATOR_EQ:
            single->set_boundary_op(yoriito::viss::v1::Operator::OPERATOR_EQ);
            break;
          case yoriito::databroker::filter::OperatorType::OPERATOR_NE:
            single->set_boundary_op(yoriito::viss::v1::Operator::OPERATOR_NE);
            break;
          case yoriito::databroker::filter::OperatorType::OPERATOR_GT:
            single->set_boundary_op(yoriito::viss::v1::Operator::OPERATOR_GT);
            break;
          case yoriito::databroker::filter::OperatorType::OPERATOR_GTE:
            single->set_boundary_op(yoriito::viss::v1::Operator::OPERATOR_GTE);
            break;
          case yoriito::databroker::filter::OperatorType::OPERATOR_LT:
            single->set_boundary_op(yoriito::viss::v1::Operator::OPERATOR_LT);
            break;
          case yoriito::databroker::filter::OperatorType::OPERATOR_LTE:
            single->set_boundary_op(yoriito::viss::v1::Operator::OPERATOR_LTE);
            break;
          default:
            break;
        }
        single->set_boundary(single_condition->GetBoundary());
      } else {
        auto condition_array = range_filter.GetConditionArray();
        for (const auto& condition : condition_array->GetConditions()) {
          auto range_condition = range->mutable_multiple()->add_conditions();
          range_condition->set_boundary(condition.GetBoundary());
          switch (condition.GetBoundaryOperator()) {
            case yoriito::databroker::filter::OperatorType::OPERATOR_UNDEFINED:
              range_condition->set_boundary_op(
                  yoriito::viss::v1::Operator::OPERATOR_UNDEFINED);
              break;
            case yoriito::databroker::filter::OperatorType::OPERATOR_EQ:
              range_condition->set_boundary_op(
                  yoriito::viss::v1::Operator::OPERATOR_EQ);
              break;
            case yoriito::databroker::filter::OperatorType::OPERATOR_NE:
              range_condition->set_boundary_op(
                  yoriito::viss::v1::Operator::OPERATOR_NE);
              break;
            case yoriito::databroker::filter::OperatorType::OPERATOR_GT:
              range_condition->set_boundary_op(
                  yoriito::viss::v1::Operator::OPERATOR_GT);
              break;
            case yoriito::databroker::filter::OperatorType::OPERATOR_GTE:
              range_condition->set_boundary_op(
                  yoriito::viss::v1::Operator::OPERATOR_GTE);
              break;
            case yoriito::databroker::filter::OperatorType::OPERATOR_LT:
              range_condition->set_boundary_op(
                  yoriito::viss::v1::Operator::OPERATOR_LT);
              break;
            case yoriito::databroker::filter::OperatorType::OPERATOR_LTE:
              range_condition->set_boundary_op(
                  yoriito::viss::v1::Operator::OPERATOR_LTE);
              break;
            default:
              break;
          }
        }
      }
    } else if (std::holds_alternative<
                   yoriito::databroker::filter::TimeBasedFilter>(
                   data_filter.GetMiscFilter())) {
      auto& time_filter =
          std::get<yoriito::databroker::filter::TimeBasedFilter>(
              data_filter.GetMiscFilter());
      auto time_based_filter = viss_misc_filter->mutable_timebased_parameter();

      time_based_filter->set_period(time_filter.GetPeriod());
    } else {
      // do nothing
    }
  }
  return;
}

}  // namespace yoriito::databroker
