#include "Renderer.h"
#include "DataHandler.h"

#define DKU_G_DEBUG
#include "DKUTil/GUI.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace MaxsuDetectionMeter
{
    // Simple helper function to load an image into a DX11 texture with common settings
    static bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, std::int32_t& out_width, std::int32_t& out_height)
    {
        // Load from disk into a raw RGBA buffer
        int image_width = 0;
        int image_height = 0;
        unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
        if (image_data == NULL)
            return false;

        // Create texture
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = image_width;
        desc.Height = image_height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        ID3D11Texture2D* pTexture = NULL;
        D3D11_SUBRESOURCE_DATA subResource;
        subResource.pSysMem = image_data;
        subResource.SysMemPitch = desc.Width * 4;
        subResource.SysMemSlicePitch = 0;
        DKU_G_DEVICE->CreateTexture2D(&desc, &subResource, &pTexture);

        // Create texture view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        DKU_G_DEVICE->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
        pTexture->Release();

        out_width = image_width;
        out_height = image_height;
        stbi_image_free(image_data);

        return true;
    }

    static void ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle,std::int32_t alpha = 255, float uvs_width = 1.f)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        angle = (angle * std::numbers::pi) / 180.f;
        float cos_a = cosf(angle);
        float sin_a = sinf(angle);

        size.x = size.x * uvs_width;
        
        ImVec2 pos[4] =
        {
            center + ImRotate(ImVec2(-size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
            center + ImRotate(ImVec2(+size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
            center + ImRotate(ImVec2(+size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a),
            center + ImRotate(ImVec2(-size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a)
        };

        ImVec2 uvs[4] =
        {
            ImVec2(0.5f * (1 - uvs_width), 0.0f),
            ImVec2(0.5f * (1 + uvs_width), 0.0f),
            ImVec2(0.5f * (1 + uvs_width), 1.0f),
            ImVec2(0.5f * (1 - uvs_width), 1.0f)
        };
        
        ImU32 colour = IM_COL32(255, 255, 255, alpha);

        draw_list->AddImageQuad(tex_id, pos[0], pos[1], pos[2], pos[3], uvs[0], uvs[1], uvs[2], uvs[3], colour);
    }
   
    struct ImageSet
    {
        std::int32_t my_image_width = 0;
        std::int32_t my_image_height = 0;
        ID3D11ShaderResourceView* my_texture = nullptr;
    };

    static ImageSet meterset[MeterType::kTotal];
    
    static bool DrawSingleMeter(MeterPair& a_meterPair, const ImVec2 centerPos)
    {
        using fadeAction = MeterInfo::FadeType;
        
        auto targActor = RE::TESForm::LookupByID<RE::Actor>(a_meterPair.first);
       
        if (!MeterHandler::ShouldDisplayMeter(targActor))
            return false;
        
        auto meterObj = a_meterPair.second.load();

        if (!meterObj || meterObj->ShouldRemove())
            return false;

        auto meterHandler = MeterHandler::GetSingleton();
       
        const auto angle = meterObj->headingAngle;
        const float offsetX = meterHandler->meterSettings->radiusX.get_data() * std::sin(angle * 3.14f / 180.f);
        const float offsetY = -meterHandler->meterSettings->radiusY.get_data() * std::cos(angle * 3.14f / 180.f);
        
        for (std::uint32_t type = MeterType::kFrame; type < MeterType::kTotal; type++) {
            auto info = meterObj->infos[type];
            if (!info)
                return false;

            //---------------------------------------- Update Alpha ------------------------------------------------------------
            switch (info->alpha.GetFadeAction())
            {
            case fadeAction::KFadeIn: {
                std::int32_t alphaValue = 0;
                alphaValue = std::clamp(float(info->alpha.GetCurrentValue() + ImGui::GetIO().DeltaTime * meterHandler->meterSettings->fadeSpeed.get_data()), 0.f, 255.f);
                info->alpha.SetValue(alphaValue);
                break;
            }

            case fadeAction::KFadeOut: {
                std::int32_t alphaValue = 0;
                alphaValue = std::clamp(float(info->alpha.GetCurrentValue() - ImGui::GetIO().DeltaTime * meterHandler->meterSettings->fadeSpeed.get_data()), 0.f, 255.f);
                info->alpha.SetValue(alphaValue);
                break;
            }

            default:
                break;
            }
            //----------------------------------------------------------------------------------------------------------------------


            //-------------------------------------------- Update Filling ------------------------------------------------------------
            float filling = 0.f;
           
            if (abs(info->filling.GetTargetFilling() - info->filling.GetCurrentFilling()) > 0.01f) {
                float fillingDelta = info->filling.GetTargetFilling() >= 1.0f ?
                    ImGui::GetIO().DeltaTime * meterHandler->meterSettings->maxFillingSpeed.get_data() :
                    ImGui::GetIO().DeltaTime * std::clamp(abs(info->filling.GetTargetFilling() - info->filling.GetCurrentFilling()) / info->filling.GetCurrentFilling(), float(meterHandler->meterSettings->minFillingSpeed.get_data()), float(meterHandler->meterSettings->maxFillingSpeed.get_data()));

                if (info->filling.GetTargetFilling() > info->filling.GetCurrentFilling())
                    filling = info->filling.GetCurrentFilling() + fillingDelta;
                else if (info->filling.GetTargetFilling() < info->filling.GetCurrentFilling())
                    filling = info->filling.GetCurrentFilling() - fillingDelta;
            }
            else
                filling = info->filling.GetTargetFilling();
            
            filling = std::clamp(filling, 0.f, 1.f);
            info->filling.SetCurrentFilling(filling);
            //-------------------------------------------------------------------------------------------------------------------------

            //Draw Non-Flashing Meter
            ImageRotated(meterset[type].my_texture, centerPos + ImVec2(offsetX, offsetY), ImVec2(meterset[type].my_image_width, meterset[type].my_image_height), angle, info->alpha.GetCurrentValue(), info->filling.GetCurrentFilling());

            //Draw Flashing Meter
            if (info->flashing.IsFlashingStart() && info->filling.GetCurrentFilling() >= 1.0f) {
                //-------------------------------------------- Update Flashing ------------------------------------------------------------
                float flashDelta = -ImGui::GetIO().DeltaTime * meterHandler->meterSettings->flashSpeed.get_data();
                float flashValue = info->flashing.GetCurrentValue() <= 0.f ? 255.f : info->flashing.GetCurrentValue() + flashDelta;
                info->flashing.SetValue(std::clamp(std::int32_t(flashValue), 0, 255));
                float size = 1.f + ((255.f - info->flashing.GetCurrentValue()) / 255.f) * meterHandler->meterSettings->flashScale.get_data();  //The lower the aplha, the bigger the size.
                //-------------------------------------------------------------------------------------------------------------------------

                const float flash_offsetX = (meterHandler->meterSettings->radiusX.get_data() + meterHandler->meterSettings->flashShift.get_data() * (size - 1.f)) * std::sin(angle * 3.14f / 180.f);
                const float flash_offsetY = -(meterHandler->meterSettings->radiusY.get_data() + meterHandler->meterSettings->flashShift.get_data() * (size - 1.f)) * std::cos(angle * 3.14f / 180.f);

                ImageRotated(meterset[type].my_texture, centerPos + ImVec2(flash_offsetX, flash_offsetY), ImVec2(meterset[type].my_image_width, meterset[type].my_image_height) * size, angle, info->flashing.GetCurrentValue(), 1.0f);
            }
            else {
                info->flashing.SetValue(255);
            }
        }

        return true;
    }

	void Renderer::DrawMeters()
	{
        if (!ShowMeters || GetActiveWindow() != DKU_G_TARGETHWND)
            return;

        auto UI = RE::UI::GetSingleton();
        if (!UI || UI->GameIsPaused() || !UI->IsCursorHiddenWhenTopmost() || !UI->IsShowingMenus() || !UI->GetMenu<RE::HUDMenu>())
            return;

        auto camera = RE::PlayerCamera::GetSingleton();
        auto cameraState = camera ? camera->currentState : nullptr;
        if (cameraState->id != RE::CameraState::kFirstPerson && cameraState->id != RE::CameraState::kThirdPerson)
            return;

        auto ctrlMap = RE::ControlMap::GetSingleton();
        if (!ctrlMap || !ctrlMap->IsSneakingControlsEnabled() || !ctrlMap->IsMovementControlsEnabled() || ctrlMap->contextPriorityStack.back() != RE::UserEvents::INPUT_CONTEXT_ID::kGameplay)
            return;

        auto meterHandler = MeterHandler::GetSingleton();
        if (!meterHandler)
            return;

        auto playerRef = RE::PlayerCharacter::GetSingleton();
        if (!playerRef || !playerRef->IsSneaking()){
            meterHandler->meterArr.clear();
            return;
        }

        static constexpr ImGuiWindowFlags windowFlag = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs;

        float screenSizeX = ImGui::GetIO().DisplaySize.x, screenSizeY = ImGui::GetIO().DisplaySize.y;

        ImGui::SetNextWindowSize(ImVec2(screenSizeX, screenSizeY));
        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));

        ImGui::Begin("Maxsu_DetectionMeter",nullptr, windowFlag);

        auto const centerPos = ImVec2(
            0.5f * std::abs(screenSizeX) + meterHandler->meterSettings->centerOffsetX.get_data(),
            0.5f * std::abs(screenSizeY) + meterHandler->meterSettings->centerOffsetY.get_data()
        );

        std::scoped_lock lock(meterHandler->m_mutex);   //thread mutex lock
        auto it = meterHandler->meterArr.begin();
        while (it != meterHandler->meterArr.end()){
            if (DrawSingleMeter(*it, centerPos))
                it++;
            else
                it = meterHandler->meterArr.erase(it);
        }

        ImGui::End();
	}

    void Renderer::MessageCallback(SKSE::MessagingInterface::Message* msg)  //CallBack & LoadTextureFromFile should called after resource loaded.
    {
        if (msg->type == SKSE::MessagingInterface::kInputLoaded){
            DKUtil::GUI::InitD3D();    //Init d3d11 right before the main menu opened.
            DKUtil::GUI::AddCallback(FUNC_INFO(DrawMeters));

            INFO("GUI Init!"sv);
        }
        else if(msg->type == SKSE::MessagingInterface::kDataLoaded){
            // Read Texture only after game engine finished load all it renderer resource.
            std::string textureName[MeterType::kTotal] = { "Meter_Frame.png", "Meter_NonHostile.png", "Meter_Hostile.png" };
            for (std::int32_t i = MeterType::kFrame; i < MeterType::kTotal; i++) {
                const std::string texturePath = "Data\\SKSE\\Plugins\\MaxsuDetectionMeter\\" + textureName[i];
                if(LoadTextureFromFile(texturePath.c_str(), &meterset[i].my_texture, meterset[i].my_image_width, meterset[i].my_image_height))
                    INFO("Loaded Texture File \"{}\""sv, texturePath.c_str());
                else{
                    ERROR("Fail to load texture file \"{}\""sv, texturePath.c_str());
                    return;
                }

                meterset[i].my_image_width *= GetResolutionScaleWidth();
                meterset[i].my_image_height *= GetResolutionScaleHeight();
            }
            
            auto meterHandler = MeterHandler::GetSingleton();
            if (meterHandler->meterSettings->enableDebugLog.get_data()){
                spdlog::set_level(spdlog::level::debug);
                logger::debug("Enable Debug Log!");
            }

            ShowMeters = true;
        }
        else if (msg->type == SKSE::MessagingInterface::kPreLoadGame){
            DKUtil::GUI::RemoveCallback(FUNC_INFO(DrawMeters));

            auto meterHandler = MeterHandler::GetSingleton();
            if (meterHandler) {
                std::scoped_lock lock(meterHandler->m_mutex);   //thread mutex lock
                meterHandler->meterArr.clear();
            }

            DKUtil::GUI::AddCallback(FUNC_INFO(DrawMeters));
        }
    }

    bool Renderer::Install()
    {
        auto g_message = SKSE::GetMessagingInterface();
        if (!g_message)
        {
            logger::error("Messaging Interface Not Found!");
            return false;
        }

        g_message->RegisterListener(MessageCallback);

        return true;
    }


    float Renderer::GetResolutionScaleWidth()
    {
        return ImGui::GetIO().DisplaySize.x / 1920.f;
    }

    float Renderer::GetResolutionScaleHeight()
    { 
        return  ImGui::GetIO().DisplaySize.y / 1080.f;
    }
    
}