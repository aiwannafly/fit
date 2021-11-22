#include "PredictionStrategy.h"

#include <string>

#include "Factory.h"
#include "Factory.cpp"

namespace {
    Strategy *create(size_t orderNumber, TChoiceMatrix &history,
                     TScoreMap &scoreMap, TConfigs &configs) {
        return new PredictionStrategy(orderNumber, history, scoreMap, configs);
    }
}

bool predB = Factory<Strategy, std::string, size_t, TChoiceMatrix &, TScoreMap &, TConfigs &>::
getInstance()->registerCreator(predictionID, create);

// scores an expected value with variant C, variant D, then chooses the best
TChoice PredictionStrategy::getChoice() {
    TChoiceMatrix history = getHistory();
    TScoreMap scoreMap = getScoreMap();
    size_t orderNumber = getOrderNumber();
    std::vector<size_t> others;
    for (size_t i = 0; i < combLen; i++) {
        if (i != orderNumber) {
            others.push_back(i);
        }
    }
    std::map<std::array<TChoice, 2>, size_t> freqMap = {
            {{COOP, COOP}, 0},
            {{COOP, DEF},  0},
            {{DEF,  COOP}, 0},
            {{DEF,  DEF},  0}
    };
    for (const auto &comb: history) {
        freqMap[{comb[others[0]], comb[others[1]]}] += 1;
    }
    double coopExpectedValue = 0;
    double defendExpectedValue = 0;
    TChoice choices[2] = {COOP, DEF};
    for (const auto &choice1: choices) {
        for (const auto &choice2: choices) {
            std::array<TChoice, 3> comb = {COOP, COOP, COOP};
            comb[others[0]] = choice1;
            comb[others[1]] = choice2;
            coopExpectedValue += ((double) freqMap[{choice1, choice2}] / (double) history.size())
                                 * (double) scoreMap[comb][orderNumber];
            comb[orderNumber] = DEF;
            defendExpectedValue += ((double) freqMap[{choice1, choice2}] / (double) history.size())
                                   * (double) scoreMap[comb][orderNumber];
        }
    }
    if (coopExpectedValue > defendExpectedValue) {
        return COOP;
    }
    return DEF;
}