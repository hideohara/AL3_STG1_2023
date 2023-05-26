#include "GameScene.h"
#include "MathUtilityForText.h"
#include "TextureManager.h"
#include <cassert>

// コンストラクタ
GameScene::GameScene() {}

// デストラクタ
GameScene::~GameScene() {
	delete spriteBG_;    // BG
	delete modelStage_;  // ステージ
	delete modelPlayer_; // プレイヤー
	delete modelBeam_;   // ビーム
	delete modelEnemy_;  // 敵
}

// 初期化
void GameScene::Initialize() {

	srand((unsigned int)time(NULL));

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	// BG(2Dスプライト)
	textureHandleBG_ = TextureManager::Load("bg.jpg");
	spriteBG_ = Sprite::Create(textureHandleBG_, {0, 0});

	// ビュープロジェクションの初期化
	viewProjection_.translation_.y = 1;
	viewProjection_.translation_.z = -6;
	viewProjection_.Initialize();

	// ステージ
	textureHandleStage_ = TextureManager::Load("stage.jpg");
	modelStage_ = Model::Create();
	worldTransformStage_.Initialize();

	// ステージの位置を変更
	worldTransformStage_.translation_ = {0, -1.5f, 0};
	worldTransformStage_.scale_ = {4.5f, 1, 40};
	// 変換行列を更新
	worldTransformStage_.matWorld_ = MakeAffineMatrix(
	    worldTransformStage_.scale_, worldTransformStage_.rotation_,
	    worldTransformStage_.translation_);
	// 変換行列を定数バッファに転送
	worldTransformStage_.TransferMatrix();

	// プレイヤー
	textureHandlePlayer_ = TextureManager::Load("player.png");
	modelPlayer_ = Model::Create();
	worldTransformPlayer_.scale_ = {0.5f, 0.5f, 0.5f};
	worldTransformPlayer_.Initialize();

	// ビーム
	textureHandleBeam_ = TextureManager::Load("beam.png");
	modelBeam_ = Model::Create();
	worldTransformBeam_.scale_ = {0.3f, 0.3f, 0.3f};
	worldTransformBeam_.Initialize();

	// 敵
	textureHandleEnemy_ = TextureManager::Load("enemy.png");
	modelEnemy_ = Model::Create();
	worldTransformEnemy_.scale_ = {0.5f, 0.5f, 0.5f};
	worldTransformEnemy_.Initialize();

	// デッバクテキスト
	debugText_ = DebugText::GetInstance();
	debugText_->Initialize();
}

// 更新
void GameScene::Update() {
	// 各シーンの更新処理を呼び出す
	switch (sceneMode_) {
	case 0:
		GamePlayUpdate(); // ゲームプレイ更新
		break;
	}
}

// 表示
void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// 各シーンの背景２D表示を呼び出す
	switch (sceneMode_) {
	case 0:
		GamePlayDraw2DBack(); // ゲームプレイ2D背景表示
		break;
	}

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>

	// 各シーンの3D表示を呼び出す
	switch (sceneMode_) {
	case 0:
		GamePlayDraw3D(); // ゲームプレイ3D表示
		break;
	}

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	// 各シーンの近景２D表示を呼び出す
	switch (sceneMode_) {
	case 0:
		GamePlayDraw2DNear(); // ゲームプレイ2D近景表示
		break;
	}

	debugText_->DrawAll();

	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}

// —------------------------------------------
// プレイヤー
// —------------------------------------------

// プレイヤー更新
void GameScene::PlayerUpdate() {

	// 移動

	// 右へ移動
	if (input_->PushKey(DIK_RIGHT)) {
		worldTransformPlayer_.translation_.x += 0.1f;
	}

	// 左へ移動
	if (input_->PushKey(DIK_LEFT)) {
		worldTransformPlayer_.translation_.x -= 0.1f;
	}

	// 右端制限
	if (worldTransformPlayer_.translation_.x > 4) {
		worldTransformPlayer_.translation_.x = 4;
	}

	// 左端制限
	if (worldTransformPlayer_.translation_.x < -4) {
		worldTransformPlayer_.translation_.x = -4;
	}

	// 変換行列を更新
	worldTransformPlayer_.matWorld_ = MakeAffineMatrix(
	    worldTransformPlayer_.scale_, worldTransformPlayer_.rotation_,
	    worldTransformPlayer_.translation_);
	// 変換行列を定数バッファに転送
	worldTransformPlayer_.TransferMatrix();
}

// —------------------------------------------
// ビーム
// —------------------------------------------

// ビーム更新
void GameScene::BeamUpdate() {
	// ビーム移動
	BeamMove();

	// ビーム発生
	BeamBorn();

	// 変換行列を更新
	worldTransformBeam_.matWorld_ = MakeAffineMatrix(
	    worldTransformBeam_.scale_, worldTransformBeam_.rotation_,
	    worldTransformBeam_.translation_);
	// 変換行列を定数バッファに転送
	worldTransformBeam_.TransferMatrix();
}

// ビーム移動
void GameScene::BeamMove() {

	// 存在フラグが1ならば
	if (beamFlag_ == 1) {
		// ビームが奥へ移動する。
		worldTransformBeam_.translation_.z += 0.3f;
		// 回転
		worldTransformBeam_.rotation_.x += 0.1f;

		// 画面端まで移動したら
		if (worldTransformBeam_.translation_.z > 40) {
			// 存在フラグを０にする。
			beamFlag_ = 0;
		}
	}
}

// ビーム発生（発射）
void GameScene::BeamBorn() {

	// スペースキーを押したらビームを発射する
	if (input_->PushKey(DIK_SPACE)) {
		// 存在フラグが０ならば
		if (beamFlag_ == 0) {
			// ビーム座標にプレイヤー座標を代入する
			worldTransformBeam_.translation_.x = worldTransformPlayer_.translation_.x;
			worldTransformBeam_.translation_.z = worldTransformPlayer_.translation_.z;
			// 存在フラグを１にする。
			beamFlag_ = 1;
		}
	}
}

// —------------------------------------------
// 敵
// —------------------------------------------

// 敵更新
void GameScene::EnemyUpdate() {
	// 移動
	EnemyMove();

	// 発生
	EnemyBorn();

	// 変換行列を更新
	worldTransformEnemy_.matWorld_ = MakeAffineMatrix(
	    worldTransformEnemy_.scale_, worldTransformEnemy_.rotation_,
	    worldTransformEnemy_.translation_);
	// 変換行列を定数バッファに転送
	worldTransformEnemy_.TransferMatrix();
}

// 敵移動
void GameScene::EnemyMove() {
	// 存在フラグが1ならば
	if (enemyFlag_ == 1) {
		// 敵が手前へ移動する。
		worldTransformEnemy_.translation_.z -= 0.2f;
		// 回転
		worldTransformEnemy_.rotation_.x -= 0.1f;
		// 画面端まで移動したら
		if (worldTransformEnemy_.translation_.z < -5) {
			// 存在フラグを０にする。
			enemyFlag_ = 0;
		}
	}
}

// 敵発生
void GameScene::EnemyBorn() {

	// 敵が存在しなければ（存在フラグが0ならば）、
	// 存在フラグを1にして、z座標を40にする

	// 存在フラグが０ならば
	if (enemyFlag_ == 0) {

		// 乱数でＸ座標の指定
		int x = rand() % 80; // 80は4の10倍の2倍
		float x2 = (float)x / 10 - 4;
		// 10で割り、4を引く
		worldTransformEnemy_.translation_.x = x2;
		// 奥から発生
		worldTransformEnemy_.translation_.z = 40;
		// 存在フラグを１にする。
		enemyFlag_ = 1;
	}
}

// ------------------------------------------------
// 衝突判定
// ------------------------------------------------

// 衝突判定
void GameScene::Collision() {
	// 衝突判定（プレイヤーと敵）
	CollisionPlayerEnemy();
	// 衝突判定（ビームと敵）
	CollisionBeamEnemy();
}

// 衝突判定（プレイヤーと敵）
void GameScene::CollisionPlayerEnemy() {
	// 敵が存在すれば
	if (enemyFlag_ == 1) {
		// 差を求める
		float dx = abs(worldTransformPlayer_.translation_.x - worldTransformEnemy_.translation_.x);
		float dz = abs(worldTransformPlayer_.translation_.z - worldTransformEnemy_.translation_.z);

		// 衝突したら
		if (dx < 1 && dz < 1) {
			// 存在しない
			enemyFlag_ = 0;
			// ライフ
			playerLife_ -= 1;
		}
	}
}

// 衝突判定（ビームと敵）
void GameScene::CollisionBeamEnemy() {
	// ビームが存在すれば
	if (beamFlag_ == 1) {
		// 敵が存在すれば
		if (enemyFlag_ == 1) {
			// 差を求める
			float dx =
			    abs(worldTransformBeam_.translation_.x - worldTransformEnemy_.translation_.x);
			float dz =
			    abs(worldTransformBeam_.translation_.z - worldTransformEnemy_.translation_.z);

			// 衝突したら
			if (dx < 1 && dz < 1) {
				// 存在しない
				enemyFlag_ = 0;
				beamFlag_ = 0;
				// スコア加算
				gameScore_ += 1;
			}
		}
	}
}

// *************************************************
// ゲームプレイ
// *************************************************

// ゲームプレイ更新
void GameScene::GamePlayUpdate() {
	PlayerUpdate(); // プレイヤー更新
	BeamUpdate();   // ビーム更新
	EnemyUpdate();  // 敵更新
	Collision();    // 衝突判定
}
// ゲームプレイ3D描画
void GameScene::GamePlayDraw3D() {
	// ステージ
	modelStage_->Draw(worldTransformStage_, viewProjection_, textureHandleStage_);

	// プレイヤー
	modelPlayer_->Draw(worldTransformPlayer_, viewProjection_, textureHandlePlayer_);

	// ビーム
	// 存在フラグが1ならば
	if (beamFlag_ == 1) {
		modelBeam_->Draw(worldTransformBeam_, viewProjection_, textureHandleBeam_);
	}

	// 敵
	// 存在フラグが1ならば
	if (enemyFlag_ == 1) {
		modelEnemy_->Draw(worldTransformEnemy_, viewProjection_, textureHandleEnemy_);
	}
}
// ゲームプレイ2D近景描画
void GameScene::GamePlayDraw2DBack() {
	// 背景
	spriteBG_->Draw();
}

// ゲームプレイ2D近景描画
void GameScene::GamePlayDraw2DNear() {
	// ゲームスコア
	char str[100];
	sprintf_s(str, "SCORE %d", gameScore_);
	debugText_->Print(str, 200, 10, 2);

	// ライフ
	sprintf_s(str, "LIFE %d", playerLife_);
	debugText_->Print(str, 800, 10, 2);
}
