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

vector<double> optimize(vector<vector<double>> closer,unsigned int numStocks){
    double initial = 10000;
    vector<vector<double>> parts = MonteCarloWeights();
    vector<double> start;
    for (unsigned int i=0; i<numStocks;i++){
        start.push_back(closer[i][0]);
    }
    vector<vector<double>> cash;
    for (unsigned int i =0; i<parts.size(); i++) {
        vector<double> helper = {};
        for (unsigned int j = 0; j < numStocks; j++) {
            helper.push_back(initial * parts[i][j]);
        }
        cash.push_back(helper);
    }

    vector<vector<double>> num_shares;

    for (unsigned int i =0; i<parts.size(); i++) {
        vector<double> helper = {};
        for (unsigned int j = 0; j < numStocks; j++) {
            double help = cash[i][j] / start[j];
            helper.push_back(help);
        }
        num_shares.push_back(helper);
    }

    vector<vector<double>> port;
    for (unsigned int k= 0; k<parts.size();k++) {
        vector<double> helper = {};
        for (unsigned int i = 0; i < closer[0].size(); i++) {
            double sum = 0;
            for (unsigned int j = 0; j < numStocks; j++) {
                sum = sum + (num_shares[k][j] * closer[j][i]);
            }
            helper.push_back(sum);
        }
        port.push_back(helper);
    }

    vector<double> sharpe;
    for (unsigned int i=0; i < port.size();i++){
        sharpe.push_back(Sharpe_Ratio(port[i]));
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
    return parts[optimized];
}

void makeCSV(string filename, vector<double> weights) {
    ofstream myFile(filename);
    myFile << "Weights" << endl;
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


    vector<double> weigh = optimize(close_prices, n);
    makeCSV("../output/weights.csv",weigh);

    return 0;
}
