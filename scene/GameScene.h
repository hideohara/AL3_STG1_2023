#pragma once

#include "Audio.h"
#include "DebugText.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "Model.h"
#include "SafeDelete.h"
#include "Sprite.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include <time.h>

/// <summary>
/// ゲームシーン
/// </summary>
class GameScene {

public: // メンバ関数
	/// <summary>
	/// コンストクラタ
	/// </summary>
	GameScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	void PlayerUpdate();         // プレイヤー更新
	void BeamUpdate();           // ビーム更新
	void BeamMove();             // ビーム移動
	void BeamBorn();             // ビーム発生
	void EnemyUpdate();          // 敵更新
	void EnemyMove();            // 敵移動
	void EnemyBorn();            // 敵発生
	void Collision();            // 衝突判定
	void CollisionPlayerEnemy(); // 衝突判定（プレイヤーと敵）
	void CollisionBeamEnemy();   // 衝突判定（ビームと敵）

	void GamePlayUpdate();     // ゲームプレイ更新
	void GamePlayDraw3D();     // ゲームプレイ3D描画
	void GamePlayDraw2DBack(); // ゲームプレイ2D背景描画
	void GamePlayDraw2DNear(); // ゲームプレイ2D近景描画

	void TitleUpdate();     // タイトル更新
	void TitleDraw2DNear(); // タイトル2D描画

	void GameOverUpdate();     // ゲームオーバ－更新
	void GameOverDraw2DNear(); // ゲームオーバ－2D描画

	void GamePlayStart(); // ゲームプレイ開始

private: // メンバ変数
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;

	/// <summary>
	/// ゲームシーン用
	/// </summary>

	// BG（スプライト）
	uint32_t textureHandleBG_ = 0;
	Sprite* spriteBG_ = nullptr;

	// ビュープロジェクション（共通）
	ViewProjection viewProjection_;

	// ステージ
	uint32_t textureHandleStage_ = 0;
	Model* modelStage_ = nullptr;
	WorldTransform worldTransformStage_;

	// プレイヤー
	uint32_t textureHandlePlayer_ = 0;
	Model* modelPlayer_ = nullptr;
	WorldTransform worldTransformPlayer_;

	// ビ－ム
	uint32_t textureHandleBeam_ = 0;
	Model* modelBeam_ = nullptr;
	WorldTransform worldTransformBeam_;

	// 敵
	uint32_t textureHandleEnemy_ = 0;
	Model* modelEnemy_ = nullptr;
	WorldTransform worldTransformEnemy_;

	int beamFlag_ = 0;  // ビーム存在フラグ（0:存在しない、1:存在する）
	int enemyFlag_ = 0; // 敵存在フラグ（0:存在しない、1:存在する）

	DebugText* debugText_ = nullptr;

	int gameScore_ = 0;  // ゲームスコア
	int playerLife_ = 3; // プレイヤーライフ

	int sceneMode_ = 1; // シーンモード（0:ゲームプレイ　1:タイトル）

	// タイトル（スプライト）
	uint32_t textureHandleTitle_ = 0;
	Sprite* spriteTitle_ = nullptr;

	// ゲームオーバー（スプライト）
	uint32_t textureHandleGameOver_ = 0;
	Sprite* spriteGameOver_ = nullptr;

	// エンター（スプライト）
	uint32_t textureHandleEnter_ = 0;
	Sprite* spriteEnter_ = nullptr;

	int gameTimer_ = 0; // ゲームタイマー
};
