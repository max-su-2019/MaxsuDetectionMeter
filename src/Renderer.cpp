#include "Renderer.h"
#include "Events.h"

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

    static void ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle,std::int32_t alpha = 255, float uvs_height = 1.f)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        angle = (angle * std::numbers::pi) / 180.f;
        float cos_a = cosf(angle);
        float sin_a = sinf(angle);

        center.y -= 0.5f * size.y *(1 - uvs_height) * cos_a;
        center.x += 0.5f * size.y * (1 - uvs_height) * sin_a;

        size.y = size.y * uvs_height;
        
        ImVec2 pos[4] =
        {
            center + ImRotate(ImVec2(-size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
            center + ImRotate(ImVec2(+size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
            center + ImRotate(ImVec2(+size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a),
            center + ImRotate(ImVec2(-size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a)
        };

        ImVec2 uvs[4] =
        {
            ImVec2(0.0f, 0.0f),
            ImVec2(1.0f, 0.0f),
            ImVec2(1.0f, uvs_height),
            ImVec2(0.0f,uvs_height)
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

    static ImageSet meterFrame, meterNonHostile;

	void Renderer::DrawMeters()
	{
        if (GetActiveWindow() != DKU_G_TARGETHWND)
            return;

        auto UI = RE::UI::GetSingleton();

        if (!UI || UI->GameIsPaused() || !UI->IsCursorHiddenWhenTopmost())
            return;

        auto camera = RE::PlayerCamera::GetSingleton();
        auto cameraRoot = camera ? camera->cameraRoot : nullptr;

        auto playerChar = RE::PlayerCharacter::GetSingleton();

        auto target = RE::Console::GetSelectedRef() ? RE::Console::GetSelectedRef()->As<RE::Actor>() : nullptr;

        if (!target || !cameraRoot || !playerChar)
            return;

        auto GetDetectionLevel = [](RE::Actor* a_owner, RE::Actor* a_target) -> int32_t {
            int32_t detectionLevel = a_owner->RequestDetectionLevel(a_target, RE::DETECTION_PRIORITY::kNormal);
            if (detectionLevel < 0) {
                detectionLevel += 100;
                return detectionLevel = min(max(detectionLevel, 0), 100);
            }
            else
                return 100;
        };

        static bool start = false;

        auto detectionLevel = GetDetectionLevel(target, playerChar);
        if (!start && target->HasLOS(playerChar) && detectionLevel > 20)
            start = true;
        if (detectionLevel < 20)
            start = false;

        static std::int32_t frame_alpha = 0;
        start ? frame_alpha += ImGui::GetIO().DeltaTime * 155.f : frame_alpha -= ImGui::GetIO().DeltaTime * 155.f;
        frame_alpha = std::clamp(frame_alpha, 0, 255);

        static std::int32_t meter_alpha = 0;

        if (detectionLevel < 100) {
            start ? meter_alpha += ImGui::GetIO().DeltaTime * 155.f : meter_alpha -= ImGui::GetIO().DeltaTime * 155.f;
            meter_alpha = std::clamp(frame_alpha, 0, 255);
        }
        else {
            static bool fadeIn = false;

            if (meter_alpha >= 255) {
                fadeIn = false;
            }
            else if (meter_alpha <= 0) {
                fadeIn = true;
            }

            fadeIn ? meter_alpha += ImGui::GetIO().DeltaTime * 400.f : meter_alpha -= ImGui::GetIO().DeltaTime * 400.f;
        }


        static constexpr ImGuiWindowFlags windowFlag = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs;
        RECT screenRect;
        IM_ASSERT(GetWindowRect(DKU_G_TARGETHWND, &screenRect));
        ImGui::SetNextWindowSize(ImVec2(std::abs(screenRect.right), std::abs(screenRect.bottom)));
        ImGui::SetNextWindowPos(ImVec2(screenRect.left, screenRect.top));

        static const auto radiusY = 300.f;
        static const auto radiusX = 500.f;

        auto const centerPos = ImVec2(0.5f * std::abs(screenRect.right - screenRect.left), 0.5f * std::abs(screenRect.top - screenRect.bottom));

        
        auto CamTrans = RE::NiTransform(cameraRoot->world.rotate, playerChar->GetPosition());

        auto const angle = CamTrans.GetHeadingAngle(target->GetPosition());
        const float offsetX = radiusX * std::sin(angle * 3.14f / 180.f);
        const float offsetY = -radiusY * std::cos(angle * 3.14f / 180.f);
         

        ImGui::Begin("Maxsu_DetectionMeter",nullptr, windowFlag);

        ImageRotated((void*)meterFrame.my_texture, centerPos + ImVec2(offsetX, offsetY),ImVec2(meterFrame.my_image_width, meterFrame.my_image_height), angle, frame_alpha);

        static float filling = 0.f;

        if (filling < detectionLevel / 100.f)
            filling += ImGui::GetIO().DeltaTime * 0.25f;
        else if(filling > detectionLevel / 100.f)
            filling -= ImGui::GetIO().DeltaTime * 0.25f;

        filling = std::clamp(filling, 0.f, 100.f);

        ImageRotated((void*)meterNonHostile.my_texture, centerPos + ImVec2(offsetX, offsetY), ImVec2(meterNonHostile.my_image_width, meterNonHostile.my_image_height), angle, meter_alpha, filling);
        
        ImGui::End();
	}

    void Renderer::AddCallback(SKSE::MessagingInterface::Message* msg)  //CallBack & LoadTextureFromFile should called after resource loaded.
    {
        if (msg->type == SKSE::MessagingInterface::kDataLoaded)
        {
            
            bool ret1 = LoadTextureFromFile("Data\\SKSE\\Plugins\\Meter_NonHostile.png", &meterNonHostile.my_texture, meterNonHostile.my_image_width, meterNonHostile.my_image_height);
            IM_ASSERT(ret1);

            bool ret2 = LoadTextureFromFile("Data\\SKSE\\Plugins\\Meter_Frame.png", &meterFrame.my_texture, meterFrame.my_image_width, meterFrame.my_image_height);
            IM_ASSERT(ret2);

            DKUtil::GUI::AddCallback(FUNC_INFO(DrawMeters));
        }
    }

    bool Renderer::Install()
    {
        DKUtil::GUI::InitD3D();     //Must Call during the SKSEPlugin_Load,otherwise would freeze the game.
        DKUtil::GUI::InitImGui();    //Must Call during the SKSEPlugin_Load,otherwise would freeze the game.

        INFO("GUI Init!"sv);

        auto g_message = SKSE::GetMessagingInterface();
        if (!g_message)
        {
            logger::error("Messaging Interface Not Found!");
            return false;
        }

        g_message->RegisterListener(AddCallback);

        return true;
    }

}