// Logger.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <filesystem>

#include "log_settings.h"
#include "logthread.h"
#include "Helpers.h"

namespace fs = std::filesystem;

enum class LogMenu {
    Undefined,
    Information,
    Reset,
    PriceEvent,
    OrderEvent,
    TradeEvent,
    Quit
};

struct SimProduct {
    std::string product_id;
    double product_price;
};

SimProduct products[] =
{
    { "AAPL", 186.08 },
    { "AMD ", 124.79 },
    { "META", 283.36 },
    { "WMT ", 158.06 },
    { "JPM ", 143.07 },
    { "ORCL", 126.33 },
    { "NVDA", 431.03 },
    { "TSLA", 255.45 },
    { "TXN ", 178.70 }
};

std::vector<SimProduct> get_product_list() {
    int nproducts = sizeof(products) / sizeof(products[0]) - 1;
    int count = nproducts / 2;

    std::vector<int> indexes = randoms(count, 0, nproducts);

    std::vector<SimProduct> product_list;
    for (auto i : indexes) {
        product_list.push_back(products[i]);
    }
    return product_list;
}

LogMenu menu() {
    std::cout << std::endl;
    std::cout << "I: Information\n";
    std::cout << "R: Reset settings\n";
    std::cout << "P: Generate Price event\n";
    std::cout << "O: Generate Order event\n";
    std::cout << "T: Generate Trade event\n";
    std::cout << "Q: Quit\n";

    switch (getchar()) {
        case 'I': case 'i':
            return LogMenu::Information;
        case 'R': case 'r':
            return LogMenu::Reset;
        case 'P': case 'p':
            return LogMenu::PriceEvent;
        case 'O': case 'o':
            return LogMenu::OrderEvent;
        case 'T': case 't':
            return LogMenu::TradeEvent;
        case 'Q': case 'q':
            return LogMenu::Quit;
    }
    return LogMenu::Undefined;
}

void ShowInfo() {
    static std::string url = "https://www.google.com/search?q=%D0%BA%D0%B0%D0%BA%D0%B0%D1%8F+%D0%B5%D1%89%D1%91+%D0%B8%D0%BD%D1%84%D0%B0+%D1%82%D0%B5%D0%B1%D0%B5+%D0%BD%D1%83%D0%B6%D0%BD%D0%B0%3F";
    std::string cmd = std::string("start ").append(url);
    system(cmd.c_str());
}

void SimulatePriceEvent() {
    std::vector<SimProduct> prices = get_product_list();
    for (auto& p : prices) {
        std::vector<int> spreads = randoms(2, 0, 100);
        double price = p.product_price + (double)(spreads[0] - spreads[1]) / 100;
        log_format(LogType::Prices, "%s PRICE %.02f", p.product_id.c_str(), price);
    }
}

void SimulateOrderEvent() {
    std::vector<SimProduct> products = get_product_list();
    for (auto& p : products) {
        std::vector<int> bs_idx = randoms(1, 0, 100);
        std::vector<int> spreads = randoms(2, 0, 100);
        double delta = (spreads[0] & 1) ? (double)spreads[1] / 100 : -(double)spreads[1] / 100;

        const char* bs = (bs_idx[0] & 1) ? "BUY" : "SELL";
        double price = p.product_price + delta;
        log_format(LogType::Orders, "%s %s %.02f", p.product_id.c_str(), bs, price);
    }
}

void SimulateTradeEvent() {
    std::vector<SimProduct> products = get_product_list();
    for (auto& p : products) {
        std::vector<int> deltas = randoms(2, 0, 100);
        double price = p.product_price + (double)(deltas[0] - deltas[1]) / 100;
        log_format(LogType::Trades, "%s TRADE %.02f", p.product_id.c_str(), price);
    }
}


// 10. You should provide a way of changing a path to the root of all logs on the fly (without restarting the application).
void DoReset() {
    // For some reasons we want to change log file path
    // To do this, we have to reload settings and reopen each log streams if its filepath was changed
    // So, to get this effect we have to change the config file before reopening
    LogSettings::loadSettings();
    logManager::instance().reopen();
}

void StartLogging() {
    //  Actually, we do not start log threads until starting to log something
    log(LogType::Orders, "orders logging has started");
    log(LogType::Prices, "prices logging has started");
    log(LogType::Trades, "trades logging has started");
}

int main(int argc, char* argv[]) {
    std::string config = fs::path(argv[0]).parent_path().string() + std::string("/logger.config");

    LogSettings::loadSettings(config);
    
    // Start logging 
    StartLogging();

    for (auto quit = false; !quit; ) {
        switch (menu()) {
            case LogMenu::Information:
                std::cout << "Show info...";
                ShowInfo();
                break;
            case LogMenu::Reset:
                std::cout << "Reset...";
                DoReset();
                break;
            case LogMenu::PriceEvent:
                std::cout << "Price event...";
                SimulatePriceEvent();
                break;
            case LogMenu::OrderEvent:
                std::cout << "Order event...";
                SimulateOrderEvent();
                break;
            case LogMenu::TradeEvent:
                std::cout << "Trade event...";
                SimulateTradeEvent();
                break;
            case LogMenu::Quit:
                std::cout << "Quit...";
                quit = true;
                continue;
        }
    }
}

