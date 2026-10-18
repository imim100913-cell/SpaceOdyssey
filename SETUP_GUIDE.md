# 📘 UE5 美麗宇宙フライト 完全セットアップガイド

このガイドでは、Epic Games Launcher から Unreal Engine 5 をインストールした後に、このプロジェクトを起動して最高峰のグラフィックを構築する全手順を解説します。

---

## 1. Unreal Engine 5 のインストール（初回のみ）

1. **[Epic Games 公式サイト](https://store.epicgames.com/ja/download)** からインストーラーをダウンロードしてインストール。
2. Epic Games Launcher を開き、ログイン。
3. 左メニュー **「Unreal Engine」 ➔ 「ライブラリ」** を選択。
4. **「エンジンバージョン」** 横の **「＋」** ボタンを押し、**「5.4」**（または最新版）を選択して **「インストール」** をクリック。
   *(※Visual Studio が必要な場合は、VS2022 Community で「C++ によるゲーム開発」ワークロードをインストールしてください)*

---

## 2. プロジェクトの初回起動

1. 本フォルダにある **`SpaceOdyssey.uproject`** をダブルクリック。
2. 「Missing Modules / ビルドしますか？」というダイアログが出た場合は **「Yes」** をクリック。
   - C++コードが自動的にコンパイルされ、UE5エディタが起動します。

---

## 3. レベル（シーン）の美麗ライティング構築

エディタが開いたら、メニューの `File ➔ New Level` から **Empty Level** を作成し、`Content/Maps/SpaceFlight_Cinematic` として保存します。

### 配置する5大コアアクター

| アクター名 | 推奨パラメータ設定値 | 効果 |
| :--- | :--- | :--- |
| **Directional Light** | ・`Intensity`: **12.0 Lux**<br>・`Atmosphere Sun Light`: **有効（True）**<br>・`Cast Cloud Shadows`: **有効**<br>・`Mobility`: **Movable** | 宇宙の強烈な太陽光と大気散乱の光源 |
| **Sky Atmosphere** | ・`Planet Radius`: **6360.0 km**<br>・`Atmosphere Height`: **60.0 km**<br>・`Rayleigh Scattering`: `(0.175, 0.409, 1.0)` | 地上から宇宙へシームレスに抜ける物理大気 |
| **Sky Light** | ・`Real Time Capture`: **有効（True）**<br>・`Mobility`: **Movable** | 宇宙に出た瞬間に自然な暗黒になる環境光 |
| **Volumetric Cloud** | ・デフォルト設定を配置 | 宇宙船で突き抜けるドラマチックな雲海 |
| **PostProcessVolume** | ・`Infinite Extent (Unbound)`: **有効（全域）**<br>・`Exposure Min/Max EV100`: **0.0 / 15.0**<br>・`Bloom Method`: **Standard (Intensity: 1.5)**<br>・`Motion Blur Amount`: **0.4** | 太陽のまばゆい光と宇宙の暗闇の露出調整 |

---

## 4. 窓ガラス・地球・月の美麗マテリアル作成

### ① 動的窓ガラスマテリアル (`M_CockpitGlass`)
`Content/Materials/` にマテリアルを作成します。

- **Blend Mode**: `Translucent`
- **Shading Model**: `Default Lit` (または `Thin Translucent`)
- **Lighting Mode**: `Surface ForwardShading` (Lumenの反射を拾う設定)
- **主要パラメータ**:
  - `Base Color`: 薄いスモーキーブルー `(0.85, 0.92, 0.98)`
  - `Metallic`: `0.1`
  - `Specular`: `0.9`
  - `Roughness`: `0.05`（クリアなガラス）
  - `Opacity`: `0.15`
  - `Refraction`: `1.52`（窓ガラスのリアルな屈折）
  - `Emissive Color`: `HeatPlasmaGlow` パラメータ × オレンジ発光 `(5.0, 1.2, 0.1)`（再突入時の炎）
  - `Roughness / Opacity` のフチに `FrostAmount` パラメータで氷テクスチャをLerp合成（成層圏の霜）

---

### ② リアルな地球マテリアル (`M_EarthPlanet`)
大きな球体メッシュ（Sphere）に適用します。

1. **昼側のテクスチャ**: NASA Blue Marble などのカラーテクスチャを Base Color に接続。
2. **夜側の都市の灯り (City Lights)**:
   - 太陽光ベクトル（Light Vector）の内積（Dot Product）を取り、夜側（裏側）にだけ都市の灯りテクスチャを Emissive Color で発光させる。
3. **雲レイヤー (Clouds)**:
   - 雲テクスチャを少しだけ UV スクロール（自転・風）させ、地球表面にうっすらと雲の落影（Shadow）を落とす。
4. **大気の青いリム発光 (Atmosphere Rim Glow)**:
   - Fresnel（フレネル）ノードを使い、惑星の輪郭に美しいシアンブルーの発光を加える。

---

### ③ リアルな月面マテリアル (`M_MoonSurface`)
1. **NASA CGI Moon Kit** などの高精細ディスプレイスメント / 法線マップ（Normal Map）を適用。
2. `Roughness`: `0.95`（月面レゴリスの反射特性、ザラザラした質感）。
3. ナナイト（Nanite）を有効にしたメッシュに適用することで、巨大なクレーターの影がリアルタイムに美しく描画されます。

---

## 5. 推奨される無料アセット（マーケットプレイス / NASA）

よりリアルなビジュアルを素早く揃えるための無料リソースです：

1. **NASA 3D Resources / Visible Earth**:
   - [NASA Visible Earth (無料・商用可・超高解像度 8K/16K 地球テクスチャ)](https://visibleearth.nasa.gov/)
   - [NASA Scientific Visualization Studio (月面高精度マップ)](https://svs.gsfc.nasa.gov/)
2. **Fab / Unreal Marketplace (無料)**:
   - **Space Shuttle / Spaceship interior (SF内装・コックピット)**
   - **Automotive Materials (最高品質のガラス・金属・カーボンシェーダー)**
   - **Niagara Fluids / Spaceship Thruster VFX (ロケット噴射炎)**

---

## 6. 実行・テストプレイ

1. エディタ上部の **Play（再生）** ボタンを押します。
2. 操縦席からの景色、**「V」キー** で側面窓や頭上ドーム窓への切り替え、**「C」キー** で自動クルーズを起動し、大気圏から宇宙、そして月への壮大な景色をお楽しみください！
