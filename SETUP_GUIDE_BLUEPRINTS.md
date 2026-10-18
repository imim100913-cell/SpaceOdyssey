# 🚀 Space Odyssey — UE5 エディタ セットアップガイド

Python スクリプトで自動構築されなかった2つの機能を、UE5 エディタで設定する手順です。
**所要時間: 約5分**

---

## ステップ 1: HUD にキーヒントを描画する（3分）

### 1-1. BP_SpaceHUD を開く
1. UE5 エディタで `Content/Blueprints/BP_SpaceHUD` をダブルクリック
2. **Event Graph** タブを開く

### 1-2. ReceiveDrawHUD イベントを追加
1. Event Graph の空白部分で右クリック
2. `ReceiveDrawHUD` と検索 → 選択
3. これにより `SizeX` と `SizeY` の出力ピンが付いたイベントノードが出現

### 1-3. DrawText ノードを接続
1. `ReceiveDrawHUD` の実行ピンからドラッグ
2. `Draw Text` と検索 → 選択
3. 以下の値を設定:

| パラメータ | 値 |
|-----------|-----|
| **Text** | `Space=上昇  W/S=前後  A/D=旋回  Mouse=視点  F=自動巡行  Esc=終了` |
| **Text Color** | R=0.8, G=0.9, B=1.0, A=0.8 (水色半透明) |
| **Screen X** | 30 |
| **Screen Y** | 30 |
| **Font** | デフォルトのまま |
| **Scale** | 1.2 |

4. **Compile** → **Save** をクリック

---

## ステップ 2: 自動巡行モードを実装する（2分）

### 2-1. BP_SpaceHUD の Level Blueprint を開く
1. メニューバー → **Blueprints** → **Open Level Blueprint**

### 2-2. 自動巡行ロジックを追加
1. 以下のノードを配置:
   - `Event Tick`
   - `Get Player Pawn (0)` 
   - `Add Movement Input`
   - `Get Actor Forward Vector`

2. 接続:
   ```
   Event Tick → Add Movement Input
   Get Player Pawn → Add Movement Input (Target)
   Get Actor Forward Vector → Add Movement Input (World Direction)
   Scale Value = 1.0
   ```

3. ※注意: 常に前進するようになります。ON/OFF 切り替えが必要な場合は、`Branch` ノードと `Boolean` 変数 `bAutoCruise` を追加してください。

4. **Compile** → **Save**

---

## ステップ 3: GameMode の確認（30秒）

1. UE5 エディタで `Content/Blueprints/BP_SpaceGameMode` をダブルクリック
2. **Class Defaults** をクリック
3. 以下を確認:
   - **HUD Class** → `BP_SpaceHUD` になっているか
   - もしなっていなければ、ドロップダウンから `BP_SpaceHUD` を選択
4. **Compile** → **Save**

---

## 完了！

デスクトップの **🚀宇宙フライトをプレイ.bat** をダブルクリックすれば、
すべての改善が反映されたゲームがプレイできます。

### 操作キー一覧
| キー | 機能 |
|------|------|
| **Space** | 上昇（大気圏突破！） |
| **W / S** | 前進 / 後退 |
| **A / D** | 左右旋回 |
| **Mouse** | 視点操作（360度見回し） |
| **F** | 自動巡行モード ON/OFF |
| **H** | HUD 表示切替 |
| **Ctrl** | 下降 |
| **Esc** | ゲーム終了 |
