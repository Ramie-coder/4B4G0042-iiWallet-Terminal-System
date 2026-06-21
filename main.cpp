#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <algorithm>
#include <sstream>

// ==========================================
// 1. 類別繼承架構 (iWallet Domain)
// ==========================================
class WalletItem {
protected:
    std::string timestamp; // 交易時間 (YYYY-MM-DD)
    int amount;            // 金額
    std::string merchant;  // 交易商家/備註
public:
    WalletItem(std::string t, int a, std::string m) : timestamp(t), amount(a), merchant(m) {}
    virtual ~WalletItem() = default;

    std::string getTimestamp() const { return timestamp; }
    int getAmount() const { return amount; }
    std::string getMerchant() const { return merchant; }

    virtual void showReceipt() const = 0; 
    virtual std::string getType() const = 0;
    virtual std::string getDetailAttribute() const = 0;
};

// 衍生類別 1：iPay 信用卡消費 (支出)
class iPayTransaction : public WalletItem {
private:
    std::string creditCard; // 使用哪張卡片付款
public:
    iPayTransaction(std::string t, int a, std::string m, std::string card) 
        : WalletItem(t, a, m), creditCard(card) {}

    void showReceipt() const override {
        std::cout << " [💸 iPay 消費] 日期: " << timestamp << " | 金額: -" << amount 
                  << " | 商店: " << merchant << " | 付款卡片: " << creditCard << "\n";
    }
    std::string getType() const override { return "IPAY_SPEND"; }
    std::string getDetailAttribute() const override { return creditCard; }
};

// 衍生類別 2：雲端帳戶儲值/轉入 (收入)
class CloudTransfer : public WalletItem {
private:
    std::string bankSource; // 轉入來源銀行
public:
    CloudTransfer(std::string t, int a, std::string m, std::string bank) 
        : WalletItem(t, a, m), bankSource(bank) {}

    void showReceipt() const override {
        std::cout << " [📥 帳戶轉入] 日期: " << timestamp << " | 金額: +" << amount 
                  << " | 備註: " << merchant << " | 來源銀行: " << bankSource << "\n";
    }
    std::string getType() const override { return "CLOUD_TRF"; }
    std::string getDetailAttribute() const override { return bankSource; }
};

// ==========================================
// 2. 防呆與畫面清理工具
// ==========================================
int getValidInt() {
    int val;
    while (!(std::cin >> val) || val < 0) {
        std::cout << "❌ 錯誤：請輸入有效的正整數金額: ";
        std::cin.clear();
        std::cin.ignore(1000, '\n');
    }
    std::cin.ignore(1000, '\n');
    return val;
}

void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

// ==========================================
// 3. iWallet 核心管理器
// ==========================================
class iWalletManager {
private:
    std::vector<std::unique_ptr<WalletItem>> walletRecords;
    const std::string filename = "iwallet_data.txt";
    int balance = 45200; // 初始模擬餘額

public:
    iWalletManager() { loadFromCloud(); }
    ~iWalletManager() { syncToCloud(); }

    void loadFromCloud() {
        std::ifstream file(filename);
        if (!file.is_open()) return;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string type, time, merch, attr;
            int amt;

            std::getline(ss, type, ',');
            std::getline(ss, time, ',');
            std::string amtStr;
            std::getline(ss, amtStr, ',');
            amt = std::stoi(amtStr);
            std::getline(ss, attr, ',');
            std::getline(ss, merch, ',');

            if (type == "IPAY_SPEND") {
                walletRecords.push_back(std::make_unique<iPayTransaction>(time, amt, merch, attr));
                balance -= amt;
            } else if (type == "CLOUD_TRF") {
                walletRecords.push_back(std::make_unique<CloudTransfer>(time, amt, merch, attr));
                balance += amt;
            }
        }
        file.close();
    }

    void syncToCloud() {
        std::ofstream file(filename);
        for (const auto& item : walletRecords) {
            file << item->getType() << "," << item->getTimestamp() << ","
                 << item->getAmount() << "," << item->getDetailAttribute() << ","
                 << item->getMerchant() << "\n";
        }
        file.close();
    }

    int getBalance() const { return balance; }

    void executeTransaction(int choice) {
        std::string time, merch, attr;
        int amt;

        std::cout << "請輸入交易日期 (YYYY-MM-DD): ";
        std::cin >> time;
        std::cout << "請輸入金額 (TWD): ";
        amt = getValidInt();
        
        if (choice == 1) {
            std::cout << "請選擇付款卡片 (1.Visa 4321 / 2.MasterCard 8888): ";
            std::getline(std::cin, attr);
            attr = (attr == "2") ? "MasterCard (8888)" : "Visa (4321)";
            std::cout << "請輸入消費商店 (如: 麥當勞 / 星巴克): ";
            std::getline(std::cin, merch);
            
            walletRecords.push_back(std::make_unique<iPayTransaction>(time, amt, merch, attr));
            balance -= amt;
            std::cout << "💸 iPay 認證成功！已成功扣款 $" << amt << " TWD。\n";
        } else {
            std::cout << "請輸入轉入來源銀行 (如: 中信銀行 / 郵局): ";
            std::getline(std::cin, attr);
            std::cout << "請輸入轉入備註: ";
            std::getline(std::cin, merch);

            walletRecords.push_back(std::make_unique<CloudTransfer>(time, amt, merch, attr));
            balance += amt;
            std::cout << "📥 帳戶儲值成功！已存入 $" << amt << " TWD。\n";
        }
    }

    void showStatement() {
        if (walletRecords.empty()) {
            std::cout << " iWallet 目前無歷史交易明細。\n";
            return;
        }

        // STL 應用：排序
        std::sort(walletRecords.begin(), walletRecords.end(), [](const auto& a, const auto& b) {
            return a->getTimestamp() < b->getTimestamp();
        });

        std::cout << "\n--- 📜 iWallet 歷史消費對帳單 (依日期排序) ---\n";
        for (const auto& item : walletRecords) {
            item->showReceipt();
        }
    }

    void showAnalysis() {
        if (walletRecords.empty()) {
            std::cout << " 暫無足夠數據進行 iPay 消費分析。\n";
            return;
        }

        // STL 應用：Map 統計
        std::map<std::string, int> cardStats;
        int totalSpend = 0;

        for (const auto& item : walletRecords) {
            if (item->getType() == "IPAY_SPEND") {
                totalSpend += item->getAmount();
                cardStats[item->getDetailAttribute()] += item->getAmount();
            }
        }

        std::cout << "\n==================================================\n";
        std::cout << "       📊 iWallet 圓餅圖消費數據分析\n";
        std::cout << "==================================================\n";
        std::cout << " 💳 iPay 總消費支出: $" << totalSpend << " TWD\n";
        std::cout << "--------------------------------------------------\n";
        std::cout << " [卡片消費佔比動態圖表]\n";
        for (const auto& pair : cardStats) {
            std::cout << " * " << pair.first << ": $" << pair.second << " ";
            int bars = pair.second / 500; // 每500元畫一個方塊
            for(int i = 0; i < bars; ++i) std::cout << "■";
            std::cout << "\n";
        }
        std::cout << "==================================================\n";
    }
};

// ==========================================
// 4. 主執行程序
// ==========================================
int main() {
    iWalletManager wallet;
    int choice = 0;

    while (true) {
        std::cout << "\n==================================================\n";
        std::cout << "✨ iWallet | 數位錢包與信用卡管理系統\n";
        std::cout << "==================================================\n";
        std::cout << " [👤 帳戶持有者: Premium User]\n";
        std::cout << " [💵 錢包可用餘額: $" << wallet.getBalance() << " TWD]\n\n";
        std::cout << " 📱 --- 錢包主要卡片 (My Cards) ---\n";
        std::cout << " [💳 1] Visa 聯名卡 (末碼: 4321)  | 可用\n";
        std::cout << " [💳 2] MasterCard (末碼: 8888)  | 可用\n\n";
        std::cout << " ⚙️ --- 錢包功能選單 (Wallet Menu) ---\n";
        std::cout << "  1. iPay 快速感應消費 (Credit Card Expense)\n";
        std::cout << "  2. 📥 連結帳戶轉入资金 (Cloud Top-up)\n";
        std::cout << "  3. 📜 檢視 iWallet 歷史對帳單 (Date Sort)\n";
        std::cout << "  4. 📊 查看 iWallet 年度消費分析 (Map Analytics)\n";
        std::cout << "  5. 🔒 安全登出並同步雲端 (Sync & Exit)\n";
        std::cout << "==================================================\n";
        std::cout << "請輸入功能編號 (1-5): ";

        choice = getValidInt();
        clearScreen();

        switch (choice) {
            case 1:
            case 2:
                wallet.executeTransaction(choice);
                break;
            case 3:
                wallet.showStatement();
                break;
            case 4:
                wallet.showAnalysis();
                break;
            case 5:
                std::cout << "☁️ 正在與雲端伺服器同步加密資料...\n";
                std::cout << "🔒 安全登出成功！感謝您使用 iWallet 系統。\n";
                return 0;
            default:
                std::cout << "❌ 指令錯誤，請輸入 1 至 5 之間的數位指令。\n";
        }
    }
    return 0;
}
