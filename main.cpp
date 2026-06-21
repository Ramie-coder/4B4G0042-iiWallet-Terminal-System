#include <iostream>
#include <string>

// ==========================================
// 1. 類別繼承架構 (第 1 迭代：驗證 OOP 架構)
// ==========================================

// 抽象基礎類別
class WalletItem {
protected:
    std::string timestamp; // 交易時間
    int amount;            // 金額
    std::string merchant;  // 交易商家
public:
    WalletItem(std::string t, int a, std::string m) : timestamp(t), amount(a), merchant(m) {}
    virtual ~WalletItem() = default;

    // 純虛擬函式：強迫子類別實作多型輸出
    virtual void showReceipt() const = 0; 
};

// 衍生類別 1：消費支出
class iPayTransaction : public WalletItem {
public:
    iPayTransaction(std::string t, int a, std::string m) : WalletItem(t, a, m) {}
    
    // 覆寫父類別的虛擬函式
    void showReceipt() const override {
        std::cout << "[💸 iPay 支出] 日期: " << timestamp << " | 商店: " << merchant << " | 金額: -" << amount << " TWD\n";
    }
};

int main() {
    std::cout << "=========================================\n";
    std::cout << "     ✨ iWallet 系統 - 第 1 迭代測試\n";
    std::cout << "=========================================\n";
    
    // 測試動態多型指標
    WalletItem* item = new iPayTransaction("2026-06-21", 150, "麥當勞");
    item->showReceipt();
    
    delete item; // 釋放記憶體
    return 0;
}
