#include "std_lib_facilities.h"
#include <iostream>
#include <ios>
#include <fstream>

using namespace std;

vector<double> importCSV(string data) {
    string date, open, high, low, close, adj_close, vol, header;
    vector<double> prices;
    ifstream fileName;
    fileName.open(data);
    getline(fileName, header);
    while( fileName.good() ) {
        getline(fileName, date, ',');
        getline(fileName, open, ',');
        getline(fileName, high, ',');
        getline(fileName, low, ',');
        getline(fileName, close, ',');
        getline(fileName, adj_close, ',');
        getline(fileName, vol, '\n');
        double close_price = stod(adj_close);
        prices.push_back(close_price);
    }
    fileName.close();
    return prices;

}

vector<double> returns(vector<double> close_price) {
    vector<double> ret;
    for (unsigned int i = 1; i < close_price.size(); i++) {
        ret.push_back((close_price[i]-close_price[i-1])/close_price[i-1]);
    }
    return ret;
}

double exp_ret(vector<double> returns) {
    double sum = 0;
    for (unsigned int i = 0; i < returns.size(); i++) {
        sum += returns[i];
    }
    return sum/returns.size();
}

double volatility(vector<double> returns) {
    double var = 0;
    double mean = exp_ret(returns);
    for (unsigned int i=0; i < returns.size(); i++) {
        var += pow(returns[i]-mean, 2);
    }
    return sqrt(var/returns.size());
}

double Sharpe_Ratio(vector<double> stocks) {
    return exp_ret(returns(stocks))/volatility(returns(stocks));
}

vector<double> randomWeights(double n) {
    vector<double> weights;
    static random_device r;
    static mt19937 engine(r());
    uniform_real_distribution<double> uniform(0,1);
    double sum = 0;
    for (unsigned int i = 0; i < n; i++) {
        weights.push_back(uniform(engine));
        sum = sum + weights[i];
    }
    for (unsigned int i = 0; i < n; i++) {
        weights[i] /= sum;
    }
    return weights;
}

vector<vector<double>> MonteCarloWeights() { //Monte Carlo
    vector<vector<double>> parts;
    for (unsigned int i=0; i<10000; i++) {
        vector<double> weights = randomWeights(5);
        parts.push_back(weights);
    }
    return parts;
}

vector<vector<double>> Cash(vector<vector<double>> manyRandomWeights, unsigned int numStocks, long money) {
    vector<vector<double>> cash;
    for (unsigned int i =0; i< manyRandomWeights.size(); i++) {
        vector<double> helper;
        for (unsigned int j = 0; j < numStocks; j++) {
            helper.push_back(money * manyRandomWeights[i][j]);
        }
        cash.push_back(helper);
    }
    return cash;
}

vector<vector<double>> numberOfShares(vector<vector<double>> manyRandomWeights, unsigned int numStocks, vector<double> startPrice, vector<vector<double>> cash) {
    vector<vector<double>> num_shares;
    for (unsigned int i =0; i<manyRandomWeights.size(); i++) {
        vector<double> helper;
        for (unsigned int j = 0; j < numStocks; j++) {
            double help = cash[i][j] / startPrice[j];
            helper.push_back(help);
        }
        num_shares.push_back(helper);
    }
    return num_shares;
}

vector<vector<double>> getPortfolio(vector<vector<double>> manyRandomWeights, vector<vector<double>> prices, vector<vector<double>> num_shares) {
    vector<vector<double>> portfolio;
    for (unsigned int i = 0; i<manyRandomWeights.size();i++) {
        vector<double> helper;
        for (unsigned int j = 0; j < prices[0].size(); j++) {
            double sum = 0;
            for (unsigned int k = 0; k < 5; k++) {
                sum = sum + (num_shares[i][k] * prices[k][j]);
            }
            helper.push_back(sum);
        }
        portfolio.push_back(helper);
    }
    return portfolio;
}

vector<double> optimize(vector<vector<double>> prices,unsigned int numStocks){
    long initial = 1000000;
    vector<vector<double>> manyRandomWeights = MonteCarloWeights();
    vector<double> starting_price;
    for (unsigned int i=0; i<numStocks;i++){
        starting_price.push_back(prices[i][0]);
    }
    vector<vector<double>> cash = Cash(manyRandomWeights, 5, initial);
    vector<vector<double>> num_shares = numberOfShares(manyRandomWeights, 5, starting_price, cash);
    vector<vector<double>> portfolio = getPortfolio(manyRandomWeights, prices, num_shares);
    vector<double> sharpe;
    for (unsigned int i=0; i < manyRandomWeights.size();i++){
        sharpe.push_back(Sharpe_Ratio(manyRandomWeights[i]));
    }
    double max_sharpe;
    for (unsigned int i = 0; i < sharpe.size();i++) {
        max_sharpe = *max_element(sharpe.begin(), sharpe.end());
    }
    double optimized;
    for (unsigned int i = 0; i < sharpe.size();i++){
        if (sharpe[i] == max_sharpe){
            optimized = i;
        }
    }
    return manyRandomWeights[optimized];
}

vector<double> weightedReturns(vector<double> weights, vector<vector<double>> daily_ret) {
    vector<double> weighted_daily_ret;
    for (unsigned int i = 0; i < daily_ret[0].size(); i++) {
        double sum = 0;
        for (unsigned int j = 0; j < weights.size(); j++) {
            sum += weights[j]*daily_ret[j][i];
        }
        weighted_daily_ret.push_back(sum);
    }
    return weighted_daily_ret;
}

void makeCSV(string filename, vector<double> weights, string header) {
    ofstream myFile(filename);
    myFile << header << endl;
    for (unsigned int i=0; i<weights.size(); i++) {
        myFile << weights[i] << endl;
    }
    myFile.close();
}




int main()
{
    unsigned int n = 5;
    vector<string> dataImport;
    dataImport.push_back("../input/AAPL.csv");
    dataImport.push_back("../input/DIS.csv");
    dataImport.push_back("../input/DKNG.csv");
    dataImport.push_back("../input/MRNA.csv");
    dataImport.push_back("../input/NFLX.csv");
    vector<vector<double>> close_prices, daily_ret;
    for (unsigned int i =0; i<n; i++) {
        close_prices.push_back(importCSV(dataImport[i]));
    }

    // Create a vector that calculates the daily returns for the close prices
    for (unsigned int i = 0; i<n; i++) {
        daily_ret.push_back(returns(close_prices[i]));
    }

    vector<double> weights = optimize(close_prices, n);
    makeCSV("../output/weights.csv",weights, "Weights");
    vector<double> weight_ret = weightedReturns(weights, daily_ret);

    makeCSV("../output/weighted_ret.csv", weight_ret, "Weighted Returns");

    cout << "All Done :) " << endl;
    return 0;
}
