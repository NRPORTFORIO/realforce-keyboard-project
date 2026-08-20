c#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// 入力感度の設定（APC機能のシミュレート）
typedef enum {
    SENSITIVITY_HIGH   = 0, // 浅い押し込みで反応（高速入力用）
    SENSITIVITY_MEDIUM = 1, // 標準的な押し込みで反応
    SENSITIVITY_LOW    = 2  // 深く押し込まないと反応しない（誤入力防止用）
} sensitivity_mode_t;

// 各感度モードに対応する、判定しきい値（アナログ値の想定：0〜4095）
// 値が大きいほど、深く押されてセンサーが近づいていることを意味します
const uint16_t THRESHOLDS[] = {
    [SENSITIVITY_HIGH]   = 1500, // センサー値が1500を超えたらON
    [SENSITIVITY_MEDIUM] = 2500, // センサー値が2500を超えたらON
    [SENSITIVITY_LOW]    = 3500  // センサー値が3500を超えたらON
};

// センサー入力の状態を管理する構造体
typedef struct {
    uint8_t            input_id;         // 入力ピン/キーのID
    sensitivity_mode_t sensitivity;      // 現在の感度設定
    uint16_t           current_raw_val;  // センサーから得た現在の生のアナログ値
    bool               is_triggered;     // 入力が確定（ON）しているか
} sensor_input_t;

/**
 * @brief アナログセンサーの値と設定された感度から、入力のON/OFFを判定する関数
 * @param input センサー状態を管理する構造体へのポインタ
 * @param raw_value 最新のアナログセンサー値
 */
void evaluate_sensor_input(sensor_input_t *input, uint16_t raw_value) {
    input->current_raw_val = raw_value;
    
    // 現在の感度設定に応じたしきい値を取得
    uint16_t target_threshold = THRESHOLDS[input->sensitivity];
    
    // ヒステリシス制御（チャタリング防止）を考慮したON/OFF判定
    // 💡POINT: ONにする時とOFFにする時で判定値に差（200の遊び）を持たせることで安定させます
    if (!input->is_triggered) {
        // 現在OFFの場合：しきい値を「超えたら」ON
        if (input->current_raw_val > target_threshold) {
            input->is_triggered = true;
        }
    } else {
        // 現在ONの場合：しきい値より「ある程度下がったら」OFF
        if (input->current_raw_val < (target_threshold - 200)) {
            input->is_triggered = false;
        }
    }
}

int main(void) {
    // ID=1、感度は標準（MEDIUM）でセンサー入力を初期化
    sensor_input_t my_sensor = {
        .input_id = 1,
        .sensitivity = SENSITIVITY_MEDIUM,
        .current_raw_val = 0,
        .is_triggered = false
    };

    // 徐々に押し込まれて、また離されていくセンサーの動きを再現したデータ
    uint16_t mock_sensor_data[] = {500, 1200, 2000, 2600, 3000, 2400, 1800, 800};
    int data_size = sizeof(mock_sensor_data) / sizeof(mock_sensor_data[0]);

    printf("--- センサー感度（標準モード：しきい値2500）の判定テスト ---\n");
    for (int i = 0; i < data_size; i++) {
        evaluate_sensor_input(&my_sensor, mock_sensor_data[i]);
        printf("データ%d | センサー値: %4d | 判定結果: %s\n", 
               i + 1, my_sensor.current_raw_val, my_sensor.is_triggered ? "ON (入力あり)" : "OFF");
    }

    return 0;
}
