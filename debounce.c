c#include <stdio.h>
#include <stdbool.h>

// チャタリング防止のための設定値（ミリ秒想定のカウント数）
#define DEBOUNCE_THRESHOLD 5

// キーの入力状態を管理する構造体
typedef struct {
    bool     raw_state;       // センサー・スイッチから直接読んだ生の値（ノイズあり）
    bool     debounced_state; // 判定が確定した綺麗な値
    uint8_t  stable_count;    // 状態が何回連続で一致したかのカウンタ
} key_filter_t;

/**
 * @brief キーの入力信号からチャタリング（ノイズ）を除去する関数
 * @param filter 状態を管理する構造体へのポインタ
 * @param current_raw_input 現在の生の入力（true: 押されている / false: 離されている）
 */
void update_key_state(key_filter_t *filter, bool current_raw_input) {
    filter->raw_state = current_raw_input;

    // 現在の生入力が、これまでの確定状態と異なる場合
    if (filter->raw_state != filter->debounced_state) {
        filter->stable_count++; // 変化が本物か監視するためカウントを増やす
        
        // 設定したしきい値（例: 5回連続）を超えたら、ノイズではなく本物の入力とみなす
        if (filter->stable_count >= DEBOUNCE_THRESHOLD) {
            filter->debounced_state = filter->raw_state; // 確定状態を更新
            filter->stable_count = 0;                    // カウンタをリセット
        }
    } else {
        // 生の入力と確定状態が同じ（＝安定している）ならカウンタは0のまま
        filter->stable_count = 0;
    }
}

// 動作テスト用の簡易的なメイン関数
int main(void) {
    key_filter_t my_key = { .raw_state = false, .debounced_state = false, .stable_count = 0 };
    
    // テスト用の疑似入力データ（ノイズが混ざった入力の流れを再現）
    // 0:離している, 1:押している
    bool mock_inputs[] = {0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0};
    int data_size = sizeof(mock_inputs) / sizeof(mock_inputs[0]);

    printf("--- チャタリング防止アルゴリズムのテスト開始 ---\n");
    for (int i = 0; i < data_size; i++) {
        update_key_state(&my_key, mock_inputs[i]);
        printf("ステップ%02d | 生の入力: %d | 確定した状態: %d (カウンタ: %d)\n", 
               i + 1, my_key.raw_state, my_key.debounced_state, my_key.stable_count);
    }
    
    return 0;
}
