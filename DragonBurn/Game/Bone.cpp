#include "Bone.h"
#include "../Helpers/Logger.h"
std::optional<CBoneData> c_model_state::GetBoneLocation(int index) {
    CBoneData boneData;
    if (!memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + index * 32, boneData)) {
        Log::Debug("[DEBUG] FAIL GetBoneLocation ");
        return std::nullopt;
    }
    Log::Debug("[DEBUG] SUCCESS read GetBoneLocation ");
    return boneData;
}

std::optional<Vec3> c_model_state::GetBonePosition(int index) {
    auto boneData = GetBoneLocation(index);
    if (!boneData.has_value()) {
        Log::Debug("[DEBUG] FAIL GetBoneLocation ");
        return std::nullopt;
    }
    Log::Debug("[DEBUG] SUCCESS GetBoneLocation ");
    return boneData->Location;
}

CHitBox::CHitBox() { memset(this, 0, sizeof(CHitBox)); }

CHitBox::CHitBox(CHitBox* hitbox) {
    if (!hitbox || !memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(hitbox), *this))
        memset(this, 0, sizeof(CHitBox));
}

std::string CHitBox::Name() {
    uintptr_t namePtr;
    char name[32] = { 0 };

    if (!memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this), namePtr) ||
        !memoryManager.ReadMemory(namePtr, name, sizeof(name) - 1)) {
        return "";
    }
    return std::string(name);
}

Vec3 CHitBox::GetStart() {
    Vec3 start;
    memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + 0x18, start);
    return start;
}

Vec3 CHitBox::GetEnd() {
    Vec3 end;
    memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + 0x24, end);
    return end;
}

float CHitBox::GetRadius() {
    float radius;
    memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + 0x30, radius);
    return radius;
}

uint32_t CHitBoxes::GetHashName() {
    uint32_t hash;
    memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + 0x20, hash);
    return hash;
}

uint32_t CHitBoxes::GetHitBoxCount() {
    uint32_t count;
    memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + 0x28, count);
    return count;
}

CHitBox* CHitBoxes::GetpHitBox(int index) {
    uintptr_t arrayPtr;
    if (!memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + 0x30, arrayPtr))
        return nullptr;
    return reinterpret_cast<CHitBox*>(arrayPtr + 0x70 * index);
}

CHitBox CHitBoxes::GetHitBox(int index) {
    CHitBox hitbox;
    if (auto ptr = GetpHitBox(index)) {
        memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(ptr), hitbox);
    }
    return hitbox;
}

CHitBoxes* CRenderMesh::GetHitboxes(int n) {
    uintptr_t ptr;
    if (!memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + 0x108, ptr))
        return nullptr;
    return reinterpret_cast<CHitBoxes*>(ptr + 0x50 * n);
}

int CRenderMesh::GetHitBoxCount() {
    int count;
    memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + 0x110, count);
    return count;
}

CRenderMesh* CRenderMeshes::GetRenderMesh(int index) {
    uintptr_t addr;
    memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + 0x8 * index, addr);
    return reinterpret_cast<CRenderMesh*>(addr);
}

CRenderMeshes* CModel::GetMeshes() {
    uintptr_t meshes;
    memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + 0x78, meshes);
    return reinterpret_cast<CRenderMeshes*>(meshes);
}

uint32_t CModel::GetMeshesCount() {
    uint32_t count;
    memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + 0x70, count);
    return count;
}

CHitBox* CModel::GetpHitBox(int index) {
    // Log function entry
    Log::Debug("[DEBUG] Entering CModel::GetpHitBox for index: " + std::to_string(index));
    
    // 1. Check meshes count
    uint32_t meshCount = GetMeshesCount();
    if (meshCount == 0) {
        Log::Debug("[DEBUG] FAILED: Mesh count is 0");
        return nullptr;
    }
    Log::Debug("[DEBUG] Mesh count: " + std::to_string(meshCount));
    
    // 2. Get meshes pointer
    auto meshes = GetMeshes();
    if (!meshes) {
        Log::Debug("[DEBUG] FAILED: Meshes pointer is null");
        return nullptr;
    }
    Log::Debug("[DEBUG] Meshes pointer: " + std::to_string(reinterpret_cast<uintptr_t>(meshes)));
    
    // 3. Get first render mesh
    auto mesh = meshes->GetRenderMesh(0);
    if (!mesh) {
        Log::Debug("[DEBUG] FAILED: First render mesh is null");
        return nullptr;
    }
    Log::Debug("[DEBUG] RenderMesh pointer: " + std::to_string(reinterpret_cast<uintptr_t>(mesh)));
    
    // 4. Check hitbox count
    int hitboxCount = mesh->GetHitBoxCount();
    if (hitboxCount <= 0) {
        Log::Debug("[DEBUG] FAILED: Hitbox count is " + std::to_string(hitboxCount));
        return nullptr;
    }
    Log::Debug("[DEBUG] Hitbox count: " + std::to_string(hitboxCount));
    
    // 5. Get hitbox set
    auto hitboxSet = mesh->GetHitboxes(0);
    if (!hitboxSet) {
        Log::Debug("[DEBUG] FAILED: HitboxSet pointer is null");
        return nullptr;
    }
    Log::Debug("[DEBUG] HitboxSet pointer: " + std::to_string(reinterpret_cast<uintptr_t>(hitboxSet)));
    
    // 6. Validate index range
    uint32_t setHitboxCount = hitboxSet->GetHitBoxCount();
    if (index >= setHitboxCount) {
        Log::Debug("[DEBUG] FAILED: Index " + std::to_string(index) + 
                  " >= HitboxSet count " + std::to_string(setHitboxCount));
        return nullptr;
    }
    Log::Debug("[DEBUG] HitboxSet count: " + std::to_string(setHitboxCount));
    
    // 7. Get specific hitbox
    CHitBox* hitbox = hitboxSet->GetpHitBox(index);
    if (!hitbox) {
        Log::Debug("[DEBUG] FAILED: GetpHitBox returned null for index " + std::to_string(index));
    } else {
        Log::Debug("[DEBUG] Successfully retrieved hitbox at address: " + 
                  std::to_string(reinterpret_cast<uintptr_t>(hitbox)));
    }
    
    return hitbox;
}
c_model_state* c_game_scene_node::GetBoneArray() {
    uintptr_t addr;
    if (!memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + Offset.Pawn.BoneArray, addr))
        return nullptr;
    return reinterpret_cast<c_model_state*>(addr);
}

CModel* c_game_scene_node::GetModel() {
    uintptr_t state, model;
    if (!memoryManager.ReadMemory(reinterpret_cast<uintptr_t>(this) + Offset.Pawn.BoneArray, state) ||
        !memoryManager.ReadMemory(state + 0xA0, model)) {
        return nullptr;
    }
    return reinterpret_cast<CModel*>(model);
}

bool CBone::UpdateAllBoneData(const DWORD64& EntityPawnAddress) {
    if (!EntityPawnAddress) return false;
    this->EntityPawnAddress = EntityPawnAddress;

    uintptr_t gameSceneNodeAddr;
    if (!memoryManager.ReadMemory(EntityPawnAddress + Offset.Pawn.GameSceneNode, gameSceneNodeAddr))
        return false;

    this->GameSceneNode = reinterpret_cast<c_game_scene_node*>(gameSceneNodeAddr);

    uintptr_t boneArrayAddr;
    if (!memoryManager.ReadMemory(gameSceneNodeAddr + Offset.Pawn.BoneArray, boneArrayAddr))
        return false;

    constexpr size_t NUM_BONES = 30;

    // Read enhanced bone data (position + rotation)
    CBoneData enhancedData[NUM_BONES];
    if (!memoryManager.ReadMemory(boneArrayAddr, enhancedData, NUM_BONES * sizeof(CBoneData)))
        return false;

    // Read original bone data (position only)
    BoneJointData originalData[NUM_BONES];
    if (!memoryManager.ReadMemory(boneArrayAddr, originalData, NUM_BONES * sizeof(BoneJointData)))
        return false;

    // Clear both lists
    BonePosList.clear();
    EnhancedBoneData.clear();
    BonePosList.reserve(NUM_BONES);
    EnhancedBoneData.reserve(NUM_BONES);

    // Populate both data structures
    for (size_t i = 0; i < NUM_BONES; ++i) {
        // Get screen position
        Vec2 screenPos;
        bool visible = gGame.View.WorldToScreen(originalData[i].Pos, screenPos);

        // Store original bone data (for compatibility)
        BonePosList.push_back({ originalData[i].Pos, screenPos, visible });

        // Store enhanced bone data
        EnhancedBoneData.push_back({
            originalData[i].Pos,  // Location
            originalData[i].Scale,  // Scale
            enhancedData[i].Rotation  // Rotation
            });
    }
    return true;
}

std::optional<CBoneData> CBone::GetEnhancedBoneData(int index) const {
    if (!GameSceneNode) {
        Log::Debug("[DEBUG] FAIL GameSceneNode ");
        return std::nullopt;
    }
       
    auto modelState = GameSceneNode->GetBoneArray();
    if (!modelState) {
        Log::Debug("[DEBUG] FAIL GetBoneArray ");
        return std::nullopt;
    }
    Log::Debug("[DEBUG] SUCCESS GetBoneArray ");
    return modelState->GetBoneLocation(index);
}

bool CBone::UpdateAllBoneDataBatch(const DWORD64& EntityPawnAddress) {
	if (EntityPawnAddress == 0)
		return false;

	this->EntityPawnAddress = EntityPawnAddress;

	// BATCH READ 1: Get dependent addresses
	std::vector<std::pair<DWORD64, SIZE_T>> batch1Requests = {
		{EntityPawnAddress + Offset.Pawn.GameSceneNode, sizeof(DWORD64)},  // GameSceneNodeAddr
	};

	std::vector<BYTE> batch1Buffer(sizeof(DWORD64));

	if (!memoryManager.BatchReadMemory(batch1Requests, batch1Buffer.data())) {
		return false;
	}

	// Extract GameSceneNode address
	DWORD64 GameSceneNodeAddr;
	memcpy(&GameSceneNodeAddr, batch1Buffer.data(), sizeof(DWORD64));

	if (GameSceneNodeAddr == 0) return false;
	this->GameSceneNode = reinterpret_cast<c_game_scene_node*>(GameSceneNodeAddr);

	// BATCH READ 2: Get BoneArray address
	std::vector<std::pair<DWORD64, SIZE_T>> batch2Requests = {
		{GameSceneNodeAddr + Offset.Pawn.BoneArray, sizeof(DWORD64)}  // BoneArrayAddress
	};

	std::vector<BYTE> batch2Buffer(sizeof(DWORD64));

	if (!memoryManager.BatchReadMemory(batch2Requests, batch2Buffer.data())) {
		return false;
	}

	// Extract BoneArray address
	DWORD64 BoneArrayAddress;
	memcpy(&BoneArrayAddress, batch2Buffer.data(), sizeof(DWORD64));

	if (BoneArrayAddress == 0) return false;

	// BATCH READ 3: Read all bone data at once
	constexpr size_t NUM_BONES = 30;
	std::vector<std::pair<DWORD64, SIZE_T>> batch3Requests;
	batch3Requests.reserve(NUM_BONES);

	// Create requests for each bone (each bone is 32 bytes apart)
	for (size_t i = 0; i < NUM_BONES; ++i) {
		batch3Requests.push_back({ BoneArrayAddress + (i * 32), sizeof(BoneJointData) });
	}

	// Calculate total buffer size
	SIZE_T total_size = NUM_BONES * sizeof(BoneJointData);
	std::vector<BYTE> batch3Buffer(total_size);

	// Perform batch read for all bones
	if (!memoryManager.BatchReadMemory(batch3Requests, batch3Buffer.data())) {
		return false;
	}

	// Clear both lists
	BonePosList.clear();
	EnhancedBoneData.clear();
	BonePosList.reserve(NUM_BONES);
	EnhancedBoneData.reserve(NUM_BONES);

	// Extract bone data from buffer
	SIZE_T offset = 0;
	for (size_t i = 0; i < NUM_BONES; ++i) {
		BoneJointData bone;
		memcpy(&bone, batch3Buffer.data() + offset, sizeof(BoneJointData));
		offset += sizeof(BoneJointData);

		Vec2 ScreenPos;
		bool IsVisible = false;
		if (gGame.View.WorldToScreen(bone.Pos, ScreenPos))
			IsVisible = true;

		// Original bone position data
		this->BonePosList.push_back({ bone.Pos, ScreenPos, IsVisible });

		// Enhanced bone data
		CBoneData enhancedBone;
		enhancedBone.Location = bone.Pos;
		enhancedBone.Scale = bone.Scale;
		this->EnhancedBoneData.push_back(enhancedBone);
	}

	return !BonePosList.empty();
}

CHitBox* CBone::GetHitBox(int index) const {
    return GameSceneNode ? GameSceneNode->GetModel()->GetpHitBox(index) : nullptr;
}

std::optional<Vec3> CBone::GetBonePosition(int boneIndex) {
    if (boneIndex < 0 || boneIndex >= BonePosList.size())
        return std::nullopt;
    return BonePosList[boneIndex].Pos;
}

std::optional<Vec2> CBone::GetBoneScreenPosition(int boneIndex) {
    if (boneIndex < 0 || boneIndex >= BonePosList.size())
        return std::nullopt;
    return BonePosList[boneIndex].ScreenPos;
}

bool CBone::IsBoneVisible(int boneIndex) {
    if (boneIndex < 0 || boneIndex >= BonePosList.size())
        return false;
    return BonePosList[boneIndex].IsVisible;
}

int HitboxToBone(const std::string& box) {
    static const std::unordered_map<std::string, int> mapping = {
        {"head_0", head}, {"neck_0", neck_0}, {"pelvis", pelvis},
        {"spine_0", spine_0}, {"spine_1", spine_1}, {"spine_2", spine_2},
        {"spine_3", spine_3}, {"hand_l", hand_L}, {"hand_r", hand_R},
        {"arm_upper_l", arm_upper_L}, {"arm_lower_l", arm_lower_L},
        {"arm_upper_r", arm_upper_R}, {"arm_lower_r", arm_lower_R},
        {"leg_upper_l", leg_upper_L}, {"leg_upper_r", leg_upper_R},
        {"leg_lower_l", leg_lower_L}, {"leg_lower_r", leg_lower_R},
        {"ankle_l", ankle_L}, {"ankle_r", ankle_R}
    };
    auto it = mapping.find(box);
    return it != mapping.end() ? it->second : -1;
}

void CreateCircle(Vec3 point, Vec3 center, float radius, std::vector<Vec3>& vec, int segments) {
    vec.clear();
    Vec3 normal = VectorTransform::Extend(center, point, 1.0f);
    normal = { normal.x - center.x, normal.y - center.y, normal.z - center.z };

    // Normalize
    float len = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (len > 0) {
        normal.x /= len;
        normal.y /= len;
        normal.z /= len;
    }

    Vec3 arbitrary = (std::abs(normal.x) < 0.99f) ? Vec3{ 1, 0, 0 } : Vec3{ 0, 1, 0 };
    Vec3 u = {
        normal.y * arbitrary.z - normal.z * arbitrary.y,
        normal.z * arbitrary.x - normal.x * arbitrary.z,
        normal.x * arbitrary.y - normal.y * arbitrary.x
    };

    // Normalize u
    len = std::sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
    if (len > 0) {
        u.x /= len;
        u.y /= len;
        u.z /= len;
    }

    Vec3 v = {
        normal.y * u.z - normal.z * u.y,
        normal.z * u.x - normal.x * u.z,
        normal.x * u.y - normal.y * u.x
    };

    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);
        Vec3 circlePoint = {
            point.x + (u.x * cosA + v.x * sinA) * radius,
            point.y + (u.y * cosA + v.y * sinA) * radius,
            point.z + (u.z * cosA + v.z * sinA) * radius
        };
        vec.push_back(circlePoint);
    }
}

void DrawCapsule(Vec3 vMin, Vec3 vMax, float radius, Quaternion_t rotation,
    Vec3 origPos, ImColor color, int segments, float thickness) {

    // Use helper functions for transformations
    Vec3 bottom = VectorTransform::Rotate(vMax, rotation);
    Vec3 top = VectorTransform::Rotate(vMin, rotation);

    // Calculate direction and extend points
    float dist = VectorTransform::Distance(top, bottom);
    Vec3 point = VectorTransform::Extend(top, bottom, dist * 2);
    float radiusHalf = radius * std::cos(M_PI / 4.0f); // 45 degrees

    // Calculate endpoints for circles
    Vec3 topExtended = VectorTransform::Extend(top, bottom, -radius);
    Vec3 bottomExtended = VectorTransform::Extend(bottom, top, -radius);

    // Create circles for capsule visualization
    std::vector<Vec3> topSmallCircle, topCircle, bottomSmallCircle, bottomCircle;
    CreateCircle(VectorTransform::Extend(top, bottom, -radiusHalf), point, radiusHalf, topSmallCircle, segments);
    CreateCircle(top, point, radius, topCircle, segments);
    CreateCircle(VectorTransform::Extend(bottom, top, -radiusHalf), point, radiusHalf, bottomSmallCircle, segments);
    CreateCircle(bottom, point, radius, bottomCircle, segments);

    // Convert to screen coordinates and store as ImVec2
    std::vector<ImVec2> wtopSmallCircle, wtopCircle, wbottomSmallCircle, wbottomCircle;
    ImVec2 w2sTop, w2sBottom;

    // Get screen positions of endpoints
    Vec2 screenTop, screenBottom;
    if (gGame.View.WorldToScreen(topExtended + origPos, screenTop))
        w2sTop = { screenTop.x, screenTop.y };
    if (gGame.View.WorldToScreen(bottomExtended + origPos, screenBottom))
        w2sBottom = { screenBottom.x, screenBottom.y };

    // Convert all circle points to screen space
    auto ConvertToImVec2 = [](const Vec2& v) { return ImVec2(v.x, v.y); };

    for (int i = 0; i < segments + 1; i++) {
        Vec2 screenPos;
        if (gGame.View.WorldToScreen(topSmallCircle[i] + origPos, screenPos))
            wtopSmallCircle.push_back(ConvertToImVec2(screenPos));
        if (gGame.View.WorldToScreen(topCircle[i] + origPos, screenPos))
            wtopCircle.push_back(ConvertToImVec2(screenPos));
        if (gGame.View.WorldToScreen(bottomSmallCircle[i] + origPos, screenPos))
            wbottomSmallCircle.push_back(ConvertToImVec2(screenPos));
        if (gGame.View.WorldToScreen(bottomCircle[i] + origPos, screenPos))
            wbottomCircle.push_back(ConvertToImVec2(screenPos));
    }

    // Get ImGui draw list
    auto drawList = ImGui::GetWindowDrawList();

    // Draw connecting lines
    for (int i = 0; i < segments; ++i) {
        // Top to top circle connection
        if (i < wtopSmallCircle.size() && wtopSmallCircle[i].x != 0 && wtopSmallCircle[i].y != 0) {
            drawList->AddLine(wtopSmallCircle[i], w2sTop, color, thickness);
            if (i < wtopCircle.size())
                drawList->AddLine(wtopSmallCircle[i], wtopCircle[i], color, thickness);
        }

        // Vertical connections
        if (i < wtopCircle.size() && i < wbottomCircle.size()) {
            drawList->AddLine(wtopCircle[i], wbottomCircle[i], color, thickness);
        }

        // Bottom to bottom circle connection
        if (i < wbottomSmallCircle.size() && wbottomSmallCircle[i].x != 0 && wbottomSmallCircle[i].y != 0) {
            if (i < wbottomCircle.size())
                drawList->AddLine(wbottomCircle[i], wbottomSmallCircle[i], color, thickness);
            drawList->AddLine(wbottomSmallCircle[i], w2sBottom, color, thickness);
        }
    }

    // Draw circles
    for (int i = 0; i < segments; ++i) {
        // Top small circle
        if (i < wtopSmallCircle.size() - 1) {
            drawList->AddLine(wtopSmallCircle[i], wtopSmallCircle[i + 1], color, thickness);
        }

        // Main top circle
        if (i < wtopCircle.size() - 1) {
            drawList->AddLine(wtopCircle[i], wtopCircle[i + 1], color, thickness);
        }

        // Main bottom circle
        if (i < wbottomCircle.size() - 1) {
            drawList->AddLine(wbottomCircle[i], wbottomCircle[i + 1], color, thickness);
        }

        // Bottom small circle
        if (i < wbottomSmallCircle.size() - 1) {
            drawList->AddLine(wbottomSmallCircle[i], wbottomSmallCircle[i + 1], color, thickness);
        }
    }
}