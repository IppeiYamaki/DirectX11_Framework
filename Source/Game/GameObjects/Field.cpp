/// @file   Field.cpp
/// @brief  地形（フィールド）のGameObject派生クラスの実装
#include "Field.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/Material.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Resources/AssetManager.h"

#include <algorithm>
#include <cmath>

namespace Game {

    //============================================================
    // コンストラクタ / デストラクタ
    //============================================================

    Field::Field(const std::string& name)
        : GameObject(name)
        , m_gridSize(257)
        , m_cellSize(1.0f)
        , m_heightAmp(12.0f)
        , m_roughness(0.55f)
        , m_isSetup(false) {
        SetTag("Field");
    }

    Field::~Field() {
        // リソースはスマートポインタで管理されているため自動解放
    }

    //============================================================
    // ライフサイクル
    //============================================================

    void Field::Initialize() {
        GameObject::Initialize();
        Engine::Logger::Info("Field initialized: " + GetName());
    }

    void Field::Update(float deltaTime) {
        if (!IsActive()) return;
        GameObject::Update(deltaTime);
    }

    void Field::Render() {
        if (!IsActive() || !m_isSetup) return;

        // メッシュとマテリアルが設定されている場合に描画
        if (m_mesh && m_material && m_renderSystem && m_material->IsInitialized()) {
            // RenderSystemにRenderItemを追加して描画を依頼
            auto* tr = GetComponent<Engine::Transform>();
            if (!tr) return;

            Engine::RenderItem item{};
            item.m_mesh = m_mesh.get();
            item.m_material = m_material.get();
            item.m_world = tr->GetWorldMatrix();
            item.m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            item.m_layer = Engine::RenderLayer::Opaque;
            item.m_orderInLayer = 0;
            item.m_stateFlags = Engine::kRenderStateNone;

            m_renderSystem->AddRenderItem(item);
        }

        GameObject::Render();
    }

    void Field::OnDestroy() {
        Engine::Logger::Info("Field destroyed: " + GetName());
        
        m_mesh.reset();
        m_material.reset();
        m_heightmap.clear();
        
        GameObject::OnDestroy();
    }

    //============================================================
    // セットアップ
    //============================================================

    bool Field::Setup(ID3D11Device* device, Engine::RenderSystem* renderSystem, Engine::AssetManager* assetManager) {
        if (!device) {
            Engine::Logger::Error("Field::Setup - device is null");
            return false;
        }

        m_device = device;
        m_renderSystem = renderSystem;
        m_assetManager = assetManager;

        // 高さマップを生成してメッシュを作成
        Regenerate();

        m_isSetup = true;
        Engine::Logger::Info("Field setup completed: " + GetName());
        return true;
    }

    //============================================================
    // 地形設定
    //============================================================

    void Field::SetGridSize(int size) {
        // Diamond-Square用に2^n + 1である必要がある
        // 入力値が適切かチェック
        if (size < 3) {
            Engine::Logger::Warn("Field::SetGridSize - size must be at least 3, clamping to 3");
            size = 3;
        }
        
        // 2^n + 1 形式かどうかをチェック
        int temp = size - 1;
        bool isPowerOfTwo = (temp > 0) && ((temp & (temp - 1)) == 0);
        if (!isPowerOfTwo) {
            Engine::Logger::Warn("Field::SetGridSize - size should be 2^n + 1 (e.g., 129, 257, 513) for best results");
        }
        
        m_gridSize = size;
    }

    int Field::GetGridSize() const {
        return m_gridSize;
    }

    void Field::SetCellSize(float size) {
        if (size <= 0.0f) {
            Engine::Logger::Warn("Field::SetCellSize - size must be positive, clamping to 0.1f");
            size = 0.1f;
        }
        m_cellSize = size;
    }

    float Field::GetCellSize() const {
        return m_cellSize;
    }

    void Field::SetHeightAmplitude(float amp) {
        // Note: Regenerate() を呼び出すまで、既存の地形には反映されません
        m_heightAmp = amp;
    }

    float Field::GetHeightAmplitude() const {
        return m_heightAmp;
    }

    void Field::SetRoughness(float roughness) {
        // Note: 0.4～0.7の範囲が推奨されます
        // Note: Regenerate() を呼び出すまで、既存の地形には反映されません
        m_roughness = std::clamp(roughness, 0.0f, 1.0f);
    }

    float Field::GetRoughness() const {
        return m_roughness;
    }

    void Field::Regenerate(unsigned int seed) {
        if (!m_device) {
            Engine::Logger::Warn("Field::Regenerate - device not set, call Setup first");
            return;
        }

        // シードが0の場合はランダムなシードを使用
        unsigned int actualSeed = seed;
        if (actualSeed == 0) {
            std::random_device rd;
            actualSeed = rd();
        }

        // 高さマップを生成
        m_heightmap.resize(static_cast<size_t>(m_gridSize) * static_cast<size_t>(m_gridSize));
        GenerateHeightmap_DiamondSquare(m_heightmap, m_gridSize, m_roughness, actualSeed);

        // メッシュを生成
        CreateMeshFromHeightmap();

        Engine::Logger::Info("Field regenerated with seed: " + std::to_string(actualSeed));
    }

    //============================================================
    // 地形情報取得
    //============================================================

    float Field::GetHeightAt(float x, float z) const {
        if (m_heightmap.empty()) {
            return 0.0f;
        }

        // ワールド座標をグリッド座標に変換
        float halfWidth = (m_gridSize - 1) * m_cellSize * 0.5f;
        float localX = x + halfWidth;
        float localZ = z + halfWidth;

        // グリッドインデックスを計算
        float fx = localX / m_cellSize;
        float fz = localZ / m_cellSize;

        int ix = static_cast<int>(std::floor(fx));
        int iz = static_cast<int>(std::floor(fz));

        // 範囲チェック
        if (ix < 0 || ix >= m_gridSize - 1 || iz < 0 || iz >= m_gridSize - 1) {
            return 0.0f;
        }

        // バイリニア補間
        float tx = fx - static_cast<float>(ix);
        float tz = fz - static_cast<float>(iz);

        int idx00 = iz * m_gridSize + ix;
        int idx10 = iz * m_gridSize + (ix + 1);
        int idx01 = (iz + 1) * m_gridSize + ix;
        int idx11 = (iz + 1) * m_gridSize + (ix + 1);

        float h00 = m_heightmap[idx00];
        float h10 = m_heightmap[idx10];
        float h01 = m_heightmap[idx01];
        float h11 = m_heightmap[idx11];

        float h0 = h00 * (1.0f - tx) + h10 * tx;
        float h1 = h01 * (1.0f - tx) + h11 * tx;

        return h0 * (1.0f - tz) + h1 * tz;
    }

    void Field::SetMaterial(const std::shared_ptr<Engine::Material>& material) {
        m_material = material;
    }

    const std::shared_ptr<Engine::Mesh>& Field::GetMesh() const {
        return m_mesh;
    }

    //============================================================
    // 内部処理
    //============================================================

    void Field::GenerateHeightmap_DiamondSquare(
        std::vector<float>& heightmap,
        int gridSize,
        float roughness,
        unsigned int seed
    ) {
        std::mt19937 rng(seed);
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

        // 高さマップを初期化
        std::fill(heightmap.begin(), heightmap.end(), 0.0f);

        // 四隅に初期値を設定
        heightmap[0] = dist(rng) * m_heightAmp;
        heightmap[gridSize - 1] = dist(rng) * m_heightAmp;
        heightmap[(gridSize - 1) * gridSize] = dist(rng) * m_heightAmp;
        heightmap[(gridSize - 1) * gridSize + (gridSize - 1)] = dist(rng) * m_heightAmp;

        int stepSize = gridSize - 1;
        float scale = m_heightAmp;

        while (stepSize > 1) {
            int halfStep = stepSize / 2;

            // Diamond step（正方形の中心を計算）
            for (int z = 0; z < gridSize - 1; z += stepSize) {
                for (int x = 0; x < gridSize - 1; x += stepSize) {
                    int idx00 = z * gridSize + x;
                    int idx10 = z * gridSize + (x + stepSize);
                    int idx01 = (z + stepSize) * gridSize + x;
                    int idx11 = (z + stepSize) * gridSize + (x + stepSize);

                    float avg = (heightmap[idx00] + heightmap[idx10] + 
                                 heightmap[idx01] + heightmap[idx11]) * 0.25f;
                    
                    int centerIdx = (z + halfStep) * gridSize + (x + halfStep);
                    heightmap[centerIdx] = avg + dist(rng) * scale;
                }
            }

            // Square step（菱形の中心を計算）
            for (int z = 0; z < gridSize; z += halfStep) {
                int xStart = (z / halfStep % 2 == 0) ? halfStep : 0;
                for (int x = xStart; x < gridSize; x += stepSize) {
                    float sum = 0.0f;
                    int count = 0;

                    // 上
                    if (z >= halfStep) {
                        sum += heightmap[(z - halfStep) * gridSize + x];
                        ++count;
                    }
                    // 下
                    if (z + halfStep < gridSize) {
                        sum += heightmap[(z + halfStep) * gridSize + x];
                        ++count;
                    }
                    // 左
                    if (x >= halfStep) {
                        sum += heightmap[z * gridSize + (x - halfStep)];
                        ++count;
                    }
                    // 右
                    if (x + halfStep < gridSize) {
                        sum += heightmap[z * gridSize + (x + halfStep)];
                        ++count;
                    }

                    if (count > 0) {
                        float avg = sum / static_cast<float>(count);
                        heightmap[z * gridSize + x] = avg + dist(rng) * scale;
                    }
                }
            }

            // スケールを縮小
            scale *= roughness;
            stepSize = halfStep;
        }
    }

    bool Field::CreateMeshFromHeightmap() {
        if (!m_device || m_heightmap.empty()) {
            return false;
        }

        // 頂点データを生成
        std::vector<Vertex> vertices;
        vertices.reserve(static_cast<size_t>(m_gridSize) * static_cast<size_t>(m_gridSize));

        float halfWidth = (m_gridSize - 1) * m_cellSize * 0.5f;

        for (int z = 0; z < m_gridSize; ++z) {
            for (int x = 0; x < m_gridSize; ++x) {
                Vertex v;
                
                float worldX = x * m_cellSize - halfWidth;
                float worldZ = z * m_cellSize - halfWidth;
                float height = m_heightmap[z * m_gridSize + x];

                v.m_pos = Engine::Vector3(worldX, height, worldZ);
                v.m_normal = Engine::Vector3(0.0f, 1.0f, 0.0f); // 後で計算
                v.m_color = Engine::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
                v.m_uv = Engine::Vector2(
                    static_cast<float>(x) / static_cast<float>(m_gridSize - 1),
                    static_cast<float>(z) / static_cast<float>(m_gridSize - 1)
                );

                vertices.push_back(v);
            }
        }

        // 法線を計算
        CalculateNormals(vertices, m_gridSize);

        // インデックスデータを生成
        std::vector<std::uint32_t> indices;
        indices.reserve(static_cast<size_t>(m_gridSize - 1) * static_cast<size_t>(m_gridSize - 1) * 6);

        for (int z = 0; z < m_gridSize - 1; ++z) {
            for (int x = 0; x < m_gridSize - 1; ++x) {
                std::uint32_t topLeft = static_cast<std::uint32_t>(z * m_gridSize + x);
                std::uint32_t topRight = topLeft + 1;
                std::uint32_t bottomLeft = static_cast<std::uint32_t>((z + 1) * m_gridSize + x);
                std::uint32_t bottomRight = bottomLeft + 1;

                // 三角形1
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // 三角形2
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        // メッシュを作成
        m_mesh = std::make_shared<Engine::Mesh>();
        bool result = m_mesh->Create(
            m_device,
            vertices.data(),
            static_cast<std::uint32_t>(sizeof(Vertex)),
            static_cast<std::uint32_t>(vertices.size()),
            indices.data(),
            static_cast<std::uint32_t>(indices.size())
        );

        if (!result) {
            Engine::Logger::Error("Field::CreateMeshFromHeightmap - Failed to create mesh");
            m_mesh.reset();
            return false;
        }

        Engine::Logger::Info("Field mesh created: " + std::to_string(vertices.size()) + 
                            " vertices, " + std::to_string(indices.size()) + " indices");
        return true;
    }

    void Field::CalculateNormals(std::vector<Vertex>& vertices, int gridSize) {
        // すべての頂点の法線をリセット
        for (auto& v : vertices) {
            v.m_normal = Engine::Vector3(0.0f, 0.0f, 0.0f);
        }

        // 各三角形から法線を計算して頂点に加算
        for (int z = 0; z < gridSize - 1; ++z) {
            for (int x = 0; x < gridSize - 1; ++x) {
                int idx0 = z * gridSize + x;
                int idx1 = (z + 1) * gridSize + x;
                int idx2 = z * gridSize + (x + 1);
                int idx3 = (z + 1) * gridSize + (x + 1);

                // 三角形1: idx0, idx1, idx2
                {
                    Engine::Vector3 v0 = vertices[idx0].m_pos;
                    Engine::Vector3 v1 = vertices[idx1].m_pos;
                    Engine::Vector3 v2 = vertices[idx2].m_pos;

                    Engine::Vector3 edge1 = v1 - v0;
                    Engine::Vector3 edge2 = v2 - v0;
                    Engine::Vector3 normal = Engine::Vector3::Cross(edge1, edge2);

                    vertices[idx0].m_normal = vertices[idx0].m_normal + normal;
                    vertices[idx1].m_normal = vertices[idx1].m_normal + normal;
                    vertices[idx2].m_normal = vertices[idx2].m_normal + normal;
                }

                // 三角形2: idx2, idx1, idx3
                {
                    Engine::Vector3 v0 = vertices[idx2].m_pos;
                    Engine::Vector3 v1 = vertices[idx1].m_pos;
                    Engine::Vector3 v2 = vertices[idx3].m_pos;

                    Engine::Vector3 edge1 = v1 - v0;
                    Engine::Vector3 edge2 = v2 - v0;
                    Engine::Vector3 normal = Engine::Vector3::Cross(edge1, edge2);

                    vertices[idx2].m_normal = vertices[idx2].m_normal + normal;
                    vertices[idx1].m_normal = vertices[idx1].m_normal + normal;
                    vertices[idx3].m_normal = vertices[idx3].m_normal + normal;
                }
            }
        }

        // 法線を正規化
        for (auto& v : vertices) {
            v.m_normal = v.m_normal.Normalized();
        }
    }

} // namespace Game
