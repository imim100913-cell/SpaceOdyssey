# 🚀 Space Odyssey: Earth to Moon (地球から月へ)
### 〜低スペックPCでも快適60FPS！ブラウザ版（WebGL）＆ Unreal Engine 5 軽量版〜

[![Play in Browser](https://img.shields.io/badge/🌐_Play_in_Browser_(GitHub_Pages)-Click_Here-00b4d8?style=for-the-badge)](https://imim100913-cell.github.io/SpaceOdyssey/)
![Performance](https://img.shields.io/badge/Low_Spec_PC-60_FPS_Optimized-10b981?style=for-the-badge)

---

## 🌐 インストール不要！ブラウザで今すぐプレイ（低スペックPC完全対応）

👉 **[https://imim100913-cell.github.io/SpaceOdyssey/](https://imim100913-cell.github.io/SpaceOdyssey/)**

ローカルPCの容量を一切消費せず、ノートPCや内蔵GPU（Intel UHD Graphics / GeForce MX150等）でも滑らかに動く **WebGL (Three.js) 軽量3Dエンジン版** を搭載しています。

### 💻 低スペックPC向けの軽量化機能
1. **3段階の画質モード切替（`1`: 超軽量 / `2`: 標準 / `3`: 高画質）** ＆ **自動FPS最適化（オートスケーラー）**
   - フレームレートが低下した場合、自動的に解像度スケールとパーティクル数を調整して快適動作を維持します。
2. **外部テクスチャ通信ゼロ（プロシージャル生成）**
   - 地球の大陸・雲、月面の海とクレーター、天の川銀河をメモリ上で高速生成するため、読み込み待ち時間ゼロ・省VRAMで動作します。
3. **UE5版の超軽量コンフィグも同梱**
   - `Config/DefaultEngine.ini` および `Config/DefaultScalability.ini` にて、Forward Shading 有効化・Volumetric Cloud / GI オフ・ScreenPercentage 65%・720p 起動設定を適用済みです。

---

## 🎮 コントロール & 操作方法（画面右上に常時キーヒント表示）

| キー / マウス操作 | 操作内容 |
| :--- | :--- |
| **`C` または `F`** | **☕ 自動遊覧クルーズモード ON / OFF**（発射台 ➔ 雲海突破 ➔ 成層圏 ➔ 天の川 ➔ 月周回） |
| **`V`** | **🪟 窓・視点切り替え**（操縦席コックピット ➔ 側面展望窓 ➔ 頭上キューポラドーム ➔ 機外ロケット視点） |
| **`Space` / `Ctrl`** | **🚀 上昇（大気圏突破） / 下降** |
| **`W` / `S`** | 前進 / 減速（ピッチ・スロットル） |
| **`A` / `D`** | 左右旋回（ヨー） |
| **`Shift`** | **⚡ スーパーブースト加速** |
| **マウスドラッグ** | **👁️ 360° フリー視点見回し** |
| **`J`** | **🌙 月軌道へ即時ワープ** |
| **`R`** | **🌍 地球発射台へリセット** |
| **`H`** | **操作ヒントパネルの表示 / 非表示** |
| **`1` / `2` / `3`** | **画質プリセット切替**（`1`: 超軽量 / `2`: 標準 / `3`: 高画質） |
