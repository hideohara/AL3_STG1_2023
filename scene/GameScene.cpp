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

	delete spriteTitle_;    // タイトル
	delete spriteGameOver_; // ゲームオーバー
	delete spriteEnter_;    // エンター
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
	textureHandleStage_ = TextureManager::Load("stage2.jpg");
	modelStage_ = Model::Create();
	for (int i = 0; i < 20; i++) {
		worldTransformStage_[i].Initialize();
	}

	// ステージの位置を変更
	for (int i = 0; i < 20; i++) {
		worldTransformStage_[i].translation_ = {0, -1.5f, 2.0f * i - 5};
		worldTransformStage_[i].scale_ = {4.5f, 1, 1};
		// 変換行列を更新
		worldTransformStage_[i].matWorld_ = MakeAffineMatrix(
		    worldTransformStage_[i].scale_, worldTransformStage_[i].rotation_,
		    worldTransformStage_[i].translation_);
		// 変換行列を定数バッファに転送
		worldTransformStage_[i].TransferMatrix();
	}

	// プレイヤー
	textureHandlePlayer_ = TextureManager::Load("player.png");
	modelPlayer_ = Model::Create();
	worldTransformPlayer_.scale_ = {0.5f, 0.5f, 0.5f};
	worldTransformPlayer_.Initialize();

	// ビーム
	textureHandleBeam_ = TextureManager::Load("beam.png");
	modelBeam_ = Model::Create();
	for (int i = 0; i < 10; i++) {
		worldTransformBeam_[i].scale_ = {0.3f, 0.3f, 0.3f};
		worldTransformBeam_[i].Initialize();
	}

	// 敵
	textureHandleEnemy_ = TextureManager::Load("enemy.png");
	modelEnemy_ = Model::Create();
	for (int i = 0; i < 10; i++) {
		worldTransformEnemy_[i].scale_ = {0.5f, 0.5f, 0.5f};
		worldTransformEnemy_[i].Initialize();
	}

	// デッバクテキスト
	debugText_ = DebugText::GetInstance();
	debugText_->Initialize();

	// タイトル(2Dスプライト)
	textureHandleTitle_ = TextureManager::Load("title.png");
	spriteTitle_ = Sprite::Create(textureHandleTitle_, {0, 0});

	// ゲームオーバー(2Dスプライト)
	textureHandleGameOver_ = TextureManager::Load("gameover.png");
	spriteGameOver_ = Sprite::Create(textureHandleGameOver_, {0, 100});

	// エンター(2Dスプライト)
	textureHandleEnter_ = TextureManager::Load("enter.png");
	spriteEnter_ = Sprite::Create(textureHandleEnter_, {400, 500});

	// サウンドデータの読み込み
	soundDataHandleTitleBGM_ = audio_->LoadWave("Audio/Ring05.wav");
	soundDataHandleGamePlayBGM_ = audio_->LoadWave("Audio/Ring08.wav");
	soundDataHandleGameOverBGM_ = audio_->LoadWave("Audio/Ring09.wav");
	soundDataHandleEnemyHitSE_ = audio_->LoadWave("Audio/chord.wav");
	soundDataHandlePlayerHitSE_ = audio_->LoadWave("Audio/tada.wav");

	// タイトルＢＧＭを再生
	voiceHandleBGM_ = audio_->PlayWave(soundDataHandleTitleBGM_, true);
}

// 更新
void GameScene::Update() {
	// 各シーンの更新処理を呼び出す
	switch (sceneMode_) {
	case 0:
		GamePlayUpdate(); // ゲームプレイ更新
		break;
	case 1:
		TitleUpdate(); // タイトル更新
		break;
	case 2:
		GameOverUpdate(); // ゲームオーバー更新
		break;
	}

	// ゲームタイマー
	gameTimer_ += 1;
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
	case 2:
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
	case 2:
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
	case 1:
		TitleDraw2DNear(); // タイトル2D近景表示
		break;
	case 2:
		GamePlayDraw2DNear(); // ゲームプレイ2D近景表示
		GameOverDraw2DNear(); // ゲームオーバー2D近景表示
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
	for (int i = 0; i < 10; i++) {
		worldTransformBeam_[i].matWorld_ = MakeAffineMatrix(
		    worldTransformBeam_[i].scale_, worldTransformBeam_[i].rotation_,
		    worldTransformBeam_[i].translation_);
		// 変換行列を定数バッファに転送
		worldTransformBeam_[i].TransferMatrix();
	}
}

// ビーム移動
void GameScene::BeamMove() {
	for (int i = 0; i < 10; i++) {
		// 存在フラグが1ならば
		if (beamFlag_[i] == 1) {
			// ビームが奥へ移動する。
			worldTransformBeam_[i].translation_.z += 0.3f;
			// 回転
			worldTransformBeam_[i].rotation_.x += 0.1f;

			// 画面端まで移動したら
			if (worldTransformBeam_[i].translation_.z > 40) {
				// 存在フラグを０にする。
				beamFlag_[i] = 0;
			}
		}
	}
}

// ビーム発生（発射）
void GameScene::BeamBorn() {
	// 発射タイマーが0ならば
	if (beamTimer_ == 0) {
		// スペースキーを押したらビームを発射する
		if (input_->PushKey(DIK_SPACE)) {
			for (int i = 0; i < 10; i++) {
				// 存在フラグが０ならば
				if (beamFlag_[i] == 0) {
					// ビーム座標にプレイヤー座標を代入する
					worldTransformBeam_[i].translation_.x = worldTransformPlayer_.translation_.x;
					worldTransformBeam_[i].translation_.z = worldTransformPlayer_.translation_.z;
					// 存在フラグを１にする。
					beamFlag_[i] = 1;

					// タイマーを１にする
					beamTimer_ = 1;
					// １個発生で終了
					break;
				}
			}
		}
	} else {
		// 発射タイマーが1以上
		// 10を超えると再び発射が可能
		beamTimer_++;
		if (beamTimer_ > 10) {
			beamTimer_ = 0;
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

	// ジャンプ
	EnemyJump();

	// 変換行列を更新
	for (int i = 0; i < 10; i++) {
		worldTransformEnemy_[i].matWorld_ = MakeAffineMatrix(
		    worldTransformEnemy_[i].scale_, worldTransformEnemy_[i].rotation_,
		    worldTransformEnemy_[i].translation_);
		// 変換行列を定数バッファに転送
		worldTransformEnemy_[i].TransferMatrix();
	}
}

// 敵移動
void GameScene::EnemyMove() {
	for (int i = 0; i < 10; i++) {
		// 存在フラグが1ならば
		if (enemyFlag_[i] == 1) {
			// 敵が手前へ移動する。
			worldTransformEnemy_[i].translation_.z -= 0.1f;
			worldTransformEnemy_[i].translation_.z -= gameTimer_ / 1000.0f;
			;
			// 回転
			worldTransformEnemy_[i].rotation_.x -= 0.1f;
			// 横移動
			worldTransformEnemy_[i].translation_.x += enemySpeed_[i];
			// ステージ右端で反転
			if (worldTransformEnemy_[i].translation_.x > 4) {
				enemySpeed_[i] = -0.1f;
			}
			// ステージ左端で反転
			if (worldTransformEnemy_[i].translation_.x < -4) {
				enemySpeed_[i] = 0.1f;
			}
			// 画面端まで移動したら
			if (worldTransformEnemy_[i].translation_.z < -5) {
				// 存在フラグを０にする。
				enemyFlag_[i] = 0;
			}
		}
	}
}

// 敵発生
void GameScene::EnemyBorn() {
	// 乱数で発生
	if (rand() % 10 == 0) {

		for (int i = 0; i < 10; i++) {
			// 敵が存在しなければ（存在フラグが0ならば）、
			// 存在フラグを1にして、z座標を40にする

			// 存在フラグが０ならば
			if (enemyFlag_[i] == 0) {

				// 乱数でＸ座標の指定
				int x = rand() % 80; // 80は4の10倍の2倍
				float x2 = (float)x / 10 - 4;
				// 10で割り、4を引く
				worldTransformEnemy_[i].translation_.x = x2;
				// 奥から発生
				worldTransformEnemy_[i].translation_.z = 40;
				// Yは0
				worldTransformEnemy_[i].translation_.y = 0;
				// 敵スピード
				if (rand() % 2 == 0) {
					enemySpeed_[i] = 0.1f;
				} else {
					enemySpeed_[i] = -0.1f;
				}
				// 存在フラグを１にする。
				enemyFlag_[i] = 1;
				// ループ終了
				break;
			}
		}
	}
}

// 敵ジャンプ
void GameScene::EnemyJump() {
	// 敵でループ
	for (int i = 0; i < 10; i++) {
		// 消滅演出ならば
		if (enemyFlag_[i] == 2) {
			// 移動（Y座標に速度を加える）
			worldTransformEnemy_[i].translation_.y += enemyJumpSpeed_[i];

			// 速度を減らす
			enemyJumpSpeed_[i] -= 0.1f;

			// 斜め移動
			worldTransformEnemy_[i].translation_.x += enemySpeed_[i] * 4;

			// 下へ落ちると消滅
			if (worldTransformEnemy_[i].translation_.y < -3) {
				// 存在しない
				enemyFlag_[i] = 0;
			}
		}
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

	for (int i = 0; i < 10; i++) {

		// 敵が存在すれば
		if (enemyFlag_[i] == 1) {
			// 差を求める
			float dx =
			    abs(worldTransformPlayer_.translation_.x - worldTransformEnemy_[i].translation_.x);
			float dz =
			    abs(worldTransformPlayer_.translation_.z - worldTransformEnemy_[i].translation_.z);

			// 衝突したら
			if (dx < 1 && dz < 1) {
				// 存在しない
				enemyFlag_[i] = 0;
				// ライフ
				playerLife_ -= 1;
				// プレイヤーヒットSE
				audio_->PlayWave(soundDataHandlePlayerHitSE_);
			}
		}
	}
}

// 衝突判定（ビームと敵）
void GameScene::CollisionBeamEnemy() {
	// 敵の数ループする
	for (int e = 0; e < 10; e++) {
		// 敵が存在すれば
		if (enemyFlag_[e] == 1) {
			// ビームの数ループする
			for (int b = 0; b < 10; b++) {
				// ビームが存在すれば
				if (beamFlag_[b] == 1) {

					// 差を求める
					float dx =
					    abs(worldTransformBeam_[b].translation_.x -
					        worldTransformEnemy_[e].translation_.x);
					float dz =
					    abs(worldTransformBeam_[b].translation_.z -
					        worldTransformEnemy_[e].translation_.z);

					// 衝突したら
					if (dx < 1 && dz < 1) {
						// 存在しない
						enemyFlag_[e] = 2;
						beamFlag_[b] = 0;
						// 敵ジャンプ速度
						enemyJumpSpeed_[e] = 1;
						// スコア加算
						gameScore_ += 1;
						// 敵ヒットSE
						audio_->PlayWave(soundDataHandleEnemyHitSE_);
					}
				}
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
	StageUpdate();  // ステージ更新

	// ライフが０で
	if (playerLife_ <= 0) {
		// モードをゲームオーバーへ変更
		sceneMode_ = 2;
		// BGM切り替え
		audio_->StopWave(voiceHandleBGM_); // 現在のBGMを停止
		voiceHandleBGM_ =
		    audio_->PlayWave(soundDataHandleGameOverBGM_, true); // ゲームプレイBGMを再生
	}
}

// ゲームプレイ開始
void GameScene::GamePlayStart() {
	gameScore_ = 0;
	playerLife_ = 3;
	for (int i = 0; i < 10; i++) {
		beamFlag_[i] = 0;
	}
	for (int i = 0; i < 10; i++) {
		enemyFlag_[i] = 0;
	}
	worldTransformPlayer_.translation_.x = 0;
	PlayerUpdate();
	gameTimer_ = 0;
}

// ゲームプレイ3D描画
void GameScene::GamePlayDraw3D() {
	// ステージ
	for (int i = 0; i < 20; i++) {
		modelStage_->Draw(worldTransformStage_[i], viewProjection_, textureHandleStage_);
	}
	// プレイヤー
	modelPlayer_->Draw(worldTransformPlayer_, viewProjection_, textureHandlePlayer_);

	// ビーム
	for (int i = 0; i < 10; i++) {
		// 存在フラグが1ならば
		if (beamFlag_[i] == 1) {
			modelBeam_->Draw(worldTransformBeam_[i], viewProjection_, textureHandleBeam_);
		}
	}

	// 敵
	for (int i = 0; i < 10; i++) {
		// 存在フラグが0以外ならば
		if (enemyFlag_[i] != 0) {
			modelEnemy_->Draw(worldTransformEnemy_[i], viewProjection_, textureHandleEnemy_);
		}
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

// ステージ更新
void GameScene::StageUpdate() {
	// 各ステージでループ
	for (int i = 0; i < 20; i++) {
		// 手前に移動
		worldTransformStage_[i].translation_.z -= 0.1f;
		// 端まで来たら奥へ戻る
		if (worldTransformStage_[i].translation_.z < -5) {
			worldTransformStage_[i].translation_.z += 40;
		}
		// 変換行列を更新
		worldTransformStage_[i].matWorld_ = MakeAffineMatrix(
		    worldTransformStage_[i].scale_, worldTransformStage_[i].rotation_,
		    worldTransformStage_[i].translation_);
		// 変換行列を定数バッファに転送
		worldTransformStage_[i].TransferMatrix();
	}
}

// ******************************************
// タイトル
// ******************************************
// タイトル更新
void GameScene::TitleUpdate() {
	// エンターキーを押した瞬間
	if (input_->TriggerKey(DIK_RETURN)) {
		// モードをゲームプレイへ変更
		sceneMode_ = 0;
		// ゲームプレイ開始
		GamePlayStart();
		// BGM切り替え
		audio_->StopWave(voiceHandleBGM_); // 現在のBGMを停止
		voiceHandleBGM_ =
		    audio_->PlayWave(soundDataHandleGamePlayBGM_, true); // ゲームプレイBGMを再生
	}
}

// タイトル表示
void GameScene::TitleDraw2DNear() {
	// タイトル表示
	spriteTitle_->Draw();

	// エンター表示
	if (gameTimer_ % 40 >= 20) {
		spriteEnter_->Draw();
	}
}

// ******************************************
// ゲームオーバー
// ******************************************

// ゲームオーバー更新
void GameScene::GameOverUpdate() {
	// エンターキーを押した瞬間
	if (input_->TriggerKey(DIK_RETURN)) {
		// モードをタイトルへ変更
		sceneMode_ = 1;
		// BGM切り替え
		audio_->StopWave(voiceHandleBGM_);                                  // 現在のBGMを停止
		voiceHandleBGM_ = audio_->PlayWave(soundDataHandleTitleBGM_, true); // BGMを再生
	}
}

// ゲームオーバー表示
void GameScene::GameOverDraw2DNear() {
	// タイトル表示
	spriteGameOver_->Draw();

	// エンター表示
	if (gameTimer_ % 40 >= 20) {
		spriteEnter_->Draw();
	}
}
