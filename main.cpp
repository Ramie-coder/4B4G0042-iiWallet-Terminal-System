#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>

// ==========================================
// 1. 類別繼承架構
// ==========================================
class WalletItem {
protected:
    std::string timestamp;
    int amount;
    std::string merchant;
public:
    WalletItem(std::string t, int a, std::string m) : timestamp(t), amount(a), merchant(m) {}
    virtual ~WalletItem() = default;

    std::string getTimestamp() const { return timestamp; }
    int getAmount() const { return amount; }
    std::string getMerchant() const { return merchant; }

    virtual void showReceipt() const = 0; 
    virtual std::string getType() const = 0;
};

class iPayTransaction : public WalletItem {
public:
    iPayTransaction(std::string t, int a, std::string m) : WalletItem(t, a, m) {}
    
    void showReceipt() const override {
        std::cout << " [💸 iPay 支出] 日期: " << timestamp << " | 商店: " << merchant << " | 金額: -" << amount << " TWD\n";
    }
    std::string getType() const override { return "IPAY_SPEND"; }
};

// ==========================================
// 2. 第 2 迭代核心：整合 STL Vector 與 fstream 讀寫檔
// ==========================================
class iWalletManager {
private:
    // STL 應用：使用 vector 搭配智慧指標管理動態多型物件，避免記憶體洩漏
    std::vector<std::unique_ptr<WalletItem>> walletRecords;
    const std::string filename = "iwallet_data.txt";

public:
    iWalletManager() { loadFromCloud(); }
    ~iWalletManager() { syncToCloud(); }

    // 讀檔功能 (File Input)
    void loadFromCloud() {
        std::ifstream file(filename);
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string type, time, merch;
            int amt;

            // 解析 CSV 格式: TYPE,DATE,AMOUNT,MERCHANT
            std::getline(ss, type, ',');
            std::getline(ss, time, ',');
            std::string amtStr;
            std::getline(ss, amtStr, ',');
            amt = std::stoi(amtStr);
            std::getline(ss, merch, ',');

            if (type == "IPAY_SPEND") {
                walletRecords.push_back(std::make_unique<iPayTransaction>(time, amt, merch));
            }
        }
        file.close();
    }

    // 寫檔功能 (File Output)
    void syncToCloud() {
        std::ofstream file(filename);
        for (const auto& item : walletRecords) {
            file << item->getType() << "," 
                 << item->getTimestamp() << ","
                 << item->getAmount() << "," 
                 << item->getMerchant() << "\n";
        }
        file.close();
    }

    void addDemoData() {
        walletRecords.push_back(std::make_unique<iPayTransaction>("2026-06-20", 120, "星巴克"));
        walletRecords.push_back(std::make_unique<iPayTransaction>("2026-06-21", 150, "麥當勞"));
    }

    void showAll() {
        std::cout << "\n--- 📜 歷史消費明細 (第 2 迭代驗證) ---\n";
        for (const auto& item : walletRecords) {
            item->showReceipt();
        }
    }
};

int main() {
    std::cout << "=========================================\n";
    std::cout << "     ✨ iWallet 系統 - 第 2 迭代測試\n";
    std::cout << "=========================================\n";
    
    iWalletManager manager;
    manager.addDemoData(); // 塞入模擬測試資料
    manager.showAll();     // 展示 Vector 容器內的多型輸出
    
    std::cout << "\n💾 系統關閉，觸發解構子，自動寫入 iwallet_data.txt...\n";
    return 0;
}
