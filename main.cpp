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
// [新增功能] 模擬 AES-256 的 256-bit 密鑰流安全加密器
// ==========================================
class AES256_Mock {
private:
    // 256-bit 的自訂安全密鑰 (32字節)
    const std::vector<unsigned char> secureKey = {
        0x4A, 0x61, 0x73, 0x6F, 0x6E, 0x5F, 0x43, 0x2B,
        0x2B, 0x5F, 0x69, 0x57, 0x61, 0x6C, 0x6C, 0x65,
        0x74, 0x5F, 0x53, 0x65, 0x63, 0x75, 0x72, 0x65,
        0x4B, 0x65, 0x79, 0x5F, 0x32, 0x30, 0x32, 0x36
    };

public:
    // 檔案加密/解密核心演算法 (多輪位元混淆)
    std::string processData(const std::string& input) {
        std::string output = input;
        for (size_t i = 0; i < output.length(); ++i) {
            // 利用 256-bit 密鑰進行互斥或位元運算，並加入索引動態位移模擬區塊串接(CBC)效果
            output[i] = output[i] ^ secureKey[i % secureKey.size()] ^ (i * 13 % 256);
        }
        return output;
    }
};

// ==========================================
// 自訂例外處理類別
// ==========================================
class WalletException : public std::runtime_error {
public:
    explicit WalletException(const std::string& message) : std::runtime_error(message) {}
};

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
    virtual std::string getDetailAttribute() const = 0;
};

class iPayTransaction : public WalletItem {
private:
    std::string creditCard; 
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

class CloudTransfer : public WalletItem {
private:
    std::string bankSource; 
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

int getValidInt() {
    int val;
    while (true) {
        try {
            if (!(std::cin >> val)) {
                std::cin.clear();
                std::cin.ignore(1000, '\n');
                throw WalletException("輸入非數字字元！");
            }
            if (val < 0) {
                throw WalletException("金額不可為負數！");
            }
            std::cin.ignore(1000, '\n');
            return val;
        }
        catch (const WalletException& e) {
            std::cout << "❌ 安全防護攔截 -> 錯誤原因: " << e.what() << " 請重新輸入有效金額: ";
        }
    }
}

void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

// ==========================================
// iWallet 核心管理器
// ==========================================
class iWalletManager {
private:
    std::vector<WalletItem*> walletRecords;
    std::set<std::string> registeredCards; 
    AES256_Mock cipher; // 宣告加密物件
    const std::string filename = "iwallet_secure_data.txt";
    int balance = 45200; 

public:
    iWalletManager() { loadFromCloud(); }
    ~iWalletManager() { 
        syncToCloud(); 
        for (auto item : walletRecords) {
            delete item;
        }
    }

    // 讀檔並自動解密
    void loadFromCloud() {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) return;

        // 讀取整份被加密的文字檔
        std::string encryptedContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // 調用演算法進行資料還原
        std::string decryptedContent = cipher.processData(encryptedContent);
        std::stringstream ss(decryptedContent);
        std::string line;

        while (std::getline(ss, line)) {
            if (line.empty()) continue;
            std::stringstream lineStream(line);
            std::string type, time, merch, attr;
            int amt;

            std::getline(lineStream, type, ',');
            std::getline(lineStream, time, ',');
            std::string amtStr;
            std::getline(lineStream, amtStr, ',');
            amt = std::stoi(amtStr);
            std::getline(lineStream, attr, ',');
            std::getline(lineStream, merch, ',');

            if (type == "IPAY_SPEND") {
                walletRecords.push_back(new iPayTransaction(time, amt, merch, attr));
                registeredCards.insert(attr);
                balance -= amt;
            } else if (type == "CLOUD_TRF") {
                walletRecords.push_back(new CloudTransfer(time, amt, merch, attr));
                balance += amt;
            }
        }
    }

    // 寫檔前全面自動加密
    void syncToCloud() {
        std::stringstream ss;
        for (const auto& item : walletRecords) {
            ss << item->getType() << "," << item->getTimestamp() << ","
               << item->getAmount() << "," << item->getDetailAttribute() << ","
               << item->getMerchant() << "\n";
        }
        
        // 將明文資料加密成密文
        std::string encryptedData = cipher.processData(ss.str());

        std::ofstream file(filename, std::ios::binary);
        file << encryptedData;
        file.close();
    }

    int getBalance() const { return balance; }

    void printRegisteredCards() const {
        if (registeredCards.empty()) {
            std::cout << " [💳] 尚未綁定任何自訂卡片 (請透過功能 1 新增消費來自動綁定)\n";
        } else {
            int count = 1;
            for (const auto& card : registeredCards) {
                std::cout << " [💳 " << count++ << "] " << card << "  | 已連線可用\n";
            }
        }
    }

    void executeTransaction(int choice) {
        std::string time, merch, attr;
        int amt;

        std::cout << "請輸入交易日期 (YYYY-MM-DD): ";
        std::cin >> time;

        std::cout << "請輸入金額 (TWD): ";
        amt = getValidInt();
        
        if (choice == 1) {
            std::cout << "請輸入自訂付款卡片名稱 (如: 富邦吉鶴卡、中信 LINE Pay): ";
            std::getline(std::cin, attr);
            std::cout << "請輸入自訂消費商家/商店名稱 (如: 藏壽司、蝦皮購物): ";
            std::getline(std::cin, merch);
            
            if (balance - amt < 0) {
                std::cout << "⚠️ [iWallet 提醒] 當前餘額不足，錢包將出現負債餘額。\n";
            }
            
            walletRecords.push_back(new iPayTransaction(time, amt, merch, attr));
            registeredCards.insert(attr); 
            balance -= amt;
            std::cout << "💸 iPay 認證成功！已透過 " << attr << " 扣款 $" << amt << " TWD。\n";
        } else {
            std::cout << "請輸入轉入來源銀行 (如: 中信銀行 / 郵局): ";
            std::getline(std::cin, attr);
            std::cout << "請輸入轉入儲值備註 (如: 零用錢、薪水): ";
            std::getline(std::cin, merch);

            walletRecords.push_back(new CloudTransfer(time, amt, merch, attr));
            balance += amt;
            std::cout << "📥 帳戶儲值成功！已存入 $" << amt << " TWD。\n";
        }
    }

    void showStatement() {
        if (walletRecords.empty()) {
            std::cout << " iWallet 目前無歷史交易明細。\n";
            return;
        }

        std::sort(walletRecords.begin(), walletRecords.end(), [](const WalletItem* a, const WalletItem* b) {
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
            int bars = pair.second / 500; 
            for(int i = 0; i < bars; ++i) std::cout << "■";
            std::cout << "\n";
        }
        std::cout << "==================================================\n";
    }
};

int main() {
    iWalletManager wallet;
    int choice = 0;

    while (true) {
        std::cout << "\n==================================================\n";
        std::cout << "✨ iWallet | 數位錢包與信用卡管理系統\n";
        std::cout << "==================================================\n";
        std::cout << " [👤 帳戶持有者: Premium User]\n";
    }
