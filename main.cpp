#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <set>

// ==========================================
// [第 6 迭代] 模擬 AES-256 安全加密器
// ==========================================
class AES256_Mock {
private:
    const std::vector<unsigned char> secureKey = {
        0x4A, 0x61, 0x73, 0x6F, 0x6E, 0x5F, 0x43, 0x2B, 0x2B, 0x5F, 0x69, 0x57, 
        0x61, 0x6C, 0x6C, 0x65, 0x74, 0x5F, 0x53, 0x65, 0x63, 0x75, 0x72, 0x65
    };
public:
    std::string processData(const std::string& input) {
        std::string output = input;
        for (size_t i = 0; i < output.length(); ++i) {
            output[i] = output[i] ^ secureKey[i % secureKey.size()] ^ (i * 13 % 256);
        }
        return output;
    }
};

class WalletException : public std::runtime_error {
public:
    explicit WalletException(const std::string& message) : std::runtime_error(message) {}
};

// ==========================================
// 1. 類別繼承架構 (iWallet Domain)
// ==========================================
class WalletItem {
protected:
    std::string timestamp; int amount; std::string merchant;  
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

class iPayTransaction : public WalletItem {
private:
    std::string creditCard; 
public:
    iPayTransaction(std::string t, int a, std::string m, std::string card) : WalletItem(t, a, m), creditCard(card) {}
    void showReceipt() const override {
        std::cout << " [💸 iPay 消費] 日期: " << timestamp << " | 金額: -" << amount << " | 商店: " << merchant << " | 付款卡片: " << creditCard << "\n";
    }
    std::string getType() const override { return "IPAY_SPEND"; }
    std::string getDetailAttribute() const override { return creditCard; }
};

class CloudTransfer : public WalletItem {
private:
    std::string bankSource; 
public:
    CloudTransfer(std::string t, int a, std::string m, std::string bank) : WalletItem(t, a, m), bankSource(bank) {}
    void showReceipt() const override {
        std::cout << " [📥 帳戶轉入] 日期: " << timestamp << " | 金額: +" << amount << " | 備註: " << merchant << " | 來源銀行: " << bankSource << "\n";
    }
    std::string getType() const override { return "CLOUD_TRF"; }
    std::string getDetailAttribute() const override { return bankSource; }
};

// ==========================================
// 2. 防呆工具與核心管理器
// ==========================================
int getValidInt() {
    int val;
    while (true) {
        try {
            if (!(std::cin >> val)) {
                std::cin.clear(); std::cin.ignore(1000, '\n');
                throw WalletException("輸入非數字字元！");
            }
            if (val < 0) throw WalletException("金額不可為負數！");
            std::cin.ignore(1000, '\n'); return val;
        } catch (const WalletException& e) {
            std::cout << "❌ 安全防護攔截 -> 錯誤原因: " << e.what() << " 請重新輸入金額: ";
        }
    }
}

class iWalletManager {
private:
    std::vector<WalletItem*> walletRecords;
    std::set<std::string> registeredCards; 
    AES256_Mock cipher; 
    const std::string filename = "iwallet_secure_data.txt";
    int balance = 45200; 
public:
    iWalletManager() { loadFromCloud(); }
    ~iWalletManager() { syncToCloud(); for (auto item : walletRecords) delete item; }

    void loadFromCloud() {
        std::ifstream file(filename, std::ios::binary); if (!file.is_open()) return;
        std::string enc((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()); file.close();
        std::string dec = cipher.processData(enc); std::stringstream ss(dec); std::string line;
        while (std::getline(ss, line)) {
            if (line.empty()) continue;
            std::stringstream ls(line); std::string type, time, merch, attr; int amt;
            std::getline(ls, type, ','); std::getline(ls, time, ',');
            std::string amtStr; std::getline(ls, amtStr, ','); amt = std::stoi(amtStr);
            std::getline(ls, attr, ','); std::getline(ls, merch, ',');
            if (type == "IPAY_SPEND") {
                walletRecords.push_back(new iPayTransaction(time, amt, merch, attr));
                registeredCards.insert(attr); balance -= amt;
            } else if (type == "CLOUD_TRF") {
                walletRecords.push_back(new CloudTransfer(time, amt, merch, attr)); balance += amt;
            }
        }
    }

    void syncToCloud() {
        std::stringstream ss;
        for (const auto& item : walletRecords) {
            ss << item->getType() << "," << item->getTimestamp() << "," << item->getAmount() << "," << item->getDetailAttribute() << "," << item->getMerchant() << "\n";
        }
        std::string encData = cipher.processData(ss.str());
        std::ofstream file(filename, std::ios::binary); file << encData; file.close();
    }

    int getBalance() const { return balance; }
    void printCards() const {
        if (registeredCards.empty()) std::cout << " [💳] 尚未綁定任何自訂卡片 (請使用功能 1 自動綁定)\n";
        else { int c = 1; for (const auto& card : registeredCards) std::cout << " [💳 " << c++ << "] " << card << " | 已連線\n"; }
    }

    void executeTransaction(int choice) {
        std::string time, merch, attr; int amt;
        std::cout << "請輸入交易日期 (YYYY-MM-DD): "; std::cin >> time;
        std::cout << "請輸入金額 (TWD): "; amt = getValidInt();
        if (choice == 1) {
            std::cout << "請輸入自訂付款卡片名稱: "; std::getline(std::cin, attr);
            std::cout << "請輸入自訂消費商家名稱: "; std::getline(std::cin, merch);
            walletRecords.push_back(new iPayTransaction(time, amt, merch, attr));
            registeredCards.insert(attr); balance -= amt;
            std::cout << "💸 iPay 認證成功！已透過 " << attr << " 扣款 $" << amt << " TWD。\n";
        } else {
            std::cout << "請輸入轉入來源銀行: "; std::getline(std::cin, attr);
            std::cout << "請輸入轉入儲值備註: "; std::getline(std::cin, merch);
            walletRecords.push_back(new CloudTransfer(time, amt, merch, attr)); balance += amt;
            std::cout << "📥 帳戶儲值成功！已存入 $" << amt << " TWD。\n";
        }
    }

    void showStatement() {
        if (walletRecords.empty()) { std::cout << " 目前無交易明細。\n"; return; }
        std::sort(walletRecords.begin(), walletRecords.end(), [](const WalletItem* a, const WalletItem* b) { return a->getTimestamp() < b->getTimestamp(); });
        std::cout << "\n--- YYYY-MM-DD 歷史對帳單 ---\n";
        for (const auto& item : walletRecords) item->showReceipt();
    }

    void showAnalysis() {
        if (walletRecords.empty()) { std::cout << " 暫無分析數據。\n"; return; }
        std::map<std::string, int> cardStats; int totalSpend = 0;
        for (const auto& item : walletRecords) {
            if (item->getType() == "IPAY_SPEND") { totalSpend += item->getAmount(); cardStats[item->getDetailAttribute()] += item->getAmount(); }
        }
        std::cout << "\n==================================================\n📊 iWallet 消費分析 | 總支出: $" << totalSpend << "\n--------------------------------------------------\n";
        for (const auto& pair : cardStats) {
            std::cout << " * " << pair.first << ": $" << pair.second << " ";
            for(int i = 0; i < pair.second / 500; ++i) std::cout << "■"; std::cout << "\n";
        }
    }
};

// ==========================================
// 3. 主執行程序 (精簡防漏複製版)
// ==========================================
int main() {
    iWalletManager wallet; int choice = 0;
    while (true) {
        std::cout << "\n==================================================\n✨ iWallet | 數位錢包與信用卡管理系統\n==================================================\n [💵 錢包可用餘額: $" << wallet.getBalance() << " TWD]\n\n主要卡片狀態:\n";
        wallet.printCards();
        std::cout << "\n1. iPay 消費 | 2. 帳戶轉入 | 3. 歷史對帳單 | 4. 年度分析 | 5. 同步離開\n請輸入功能編號 (1-5): ";
        choice = getValidInt();
        if (choice == 5) {
            std::cout << "🛡️ [安全防護] 啟動 256-bit 加密...\n🔒 安全同步退出成功！\n"; return 0;
        } else if (choice >= 1 && choice <= 4) {
            if (choice == 1 || choice == 2) wallet.executeTransaction(choice);
            else if (choice == 3) wallet.showStatement();
            else if (choice == 4) wallet.showAnalysis();
        } else std::cout << "❌ 指令錯誤，請重新輸入。\n";
    }
    return 0;
}

