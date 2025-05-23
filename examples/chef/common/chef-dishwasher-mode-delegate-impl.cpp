/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/config.h>
#include <chef-dishwasher-mode-delegate-impl.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

#ifdef MATTER_DM_PLUGIN_DISHWASHER_MODE_SERVER
#include <chef-dishwasher-mode-delegate-impl.h>
using namespace chip::app::Clusters::DishwasherMode;

static std::unique_ptr<DishwasherModeDelegate> gDishwasherModeDelegate;
static std::unique_ptr<ModeBase::Instance> gDishwasherModeInstance;

CHIP_ERROR DishwasherModeDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void DishwasherModeDelegate::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR DishwasherModeDelegate::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR DishwasherModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DishwasherModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    if (tags.size() < kModeOptions[modeIndex].modeTags.size())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::copy(kModeOptions[modeIndex].modeTags.begin(), kModeOptions[modeIndex].modeTags.end(), tags.begin());
    tags.reduce_size(kModeOptions[modeIndex].modeTags.size());

    return CHIP_NO_ERROR;
}

// ModeBase::Instance * DishwasherMode::Instance()
// {
//     return gDishwasherModeInstance;
// }

void DishwasherMode::Shutdown()
{
    gDishwasherModeInstance.reset();
    gDishwasherModeDelegate.reset();
}

chip::Protocols::InteractionModel::Status chefDishwasherModeWriteCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                          const EmberAfAttributeMetadata * attributeMetadata,
                                                                          uint8_t * buffer)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1
    VerifyOrDie(gDishwasherModeInstance != nullptr);
    chip::Protocols::InteractionModel::Status ret;
    chip::AttributeId attributeId = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::DishwasherMode::Attributes::CurrentMode::Id: {
        uint8_t m = static_cast<uint8_t>(buffer[0]);
        ret       = gDishwasherModeInstance->UpdateCurrentMode(m);
        if (chip::Protocols::InteractionModel::Status::Success != ret)
        {
            ChipLogError(DeviceLayer, "Invalid Attribute Write to CurrentMode : %d", static_cast<int>(ret));
        }
    }
    break;
    default:
        ret = chip::Protocols::InteractionModel::Status::UnsupportedWrite;
        ChipLogError(DeviceLayer, "Unsupported Writng Attribute ID: %d", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

chip::Protocols::InteractionModel::Status chefDishwasherModeReadCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                         const EmberAfAttributeMetadata * attributeMetadata,
                                                                         uint8_t * buffer, uint16_t maxReadLength)
{
    VerifyOrReturnValue(maxReadLength == 1, chip::Protocols::InteractionModel::Status::ResourceExhausted);
    buffer[0] = gDishwasherModeInstance->GetCurrentMode();

    chip::Protocols::InteractionModel::Status ret = chip::Protocols::InteractionModel::Status::Success;
    chip::AttributeId attributeId                 = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::DishwasherMode::Attributes::CurrentMode::Id: {
        *buffer = gDishwasherModeInstance->GetCurrentMode();
        ChipLogDetail(DeviceLayer, "Reading DishwasherMode CurrentMode : %d", static_cast<int>(attributeId));
    }
    break;
    default:
        ret = chip::Protocols::InteractionModel::Status::UnsupportedRead;
        ChipLogDetail(DeviceLayer, "Unsupported attributeId %d from reading DishwasherMode", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

void emberAfDishwasherModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gDishwasherModeDelegate && !gDishwasherModeInstance);

    gDishwasherModeDelegate = std::make_unique<DishwasherModeDelegate>();
    gDishwasherModeInstance =
        std::make_unique<ModeBase::Instance>(gDishwasherModeDelegate.get(), endpointId, DishwasherMode::Id, 0);
    gDishwasherModeInstance->Init();
}
#endif // MATTER_DM_PLUGIN_DISHWASHER_MODE_SERVER
