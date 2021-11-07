#include "Runner.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>

#include "Factory.h"
#include "Factory.cpp"

constexpr size_t combinationsCount = 8;
constexpr char quitCommand[] = "quit";
constexpr char defaultMatrixFileName[] = "default_matrix.txt";
constexpr char modeKeySeq[] = "--mode=";
constexpr char stepsKeySeq[] = "--steps=";
constexpr char configsKeySeq[] = "--configs=";
constexpr char matrixKeySeq[] = "--matrix=";

namespace {
    bool startsWith(const std::string &string, const std::string &subString) {
        if (string.size() < subString.size()) {
            return false;
        }
        for (size_t i = 0; i < subString.size(); i++) {
            if (subString[i] != string[i]) {
                return false;
            }
        }
        return true;
    }
}

Runner::Runner(const std::vector<std::string> &params) {
    std::string modeKey = modeKeySeq;
    std::string stepsKey = stepsKeySeq;
    std::string configsKey = configsKeySeq;
    std::string matrixKey = matrixKeySeq;
    std::vector<std::string> names;
    for (const auto &param: params) {
        if (startsWith(param, modeKey)) {
            std::string modeName = param.substr(modeKey.length());
            if (modeMap_.find(modeName) == modeMap_.end()) {
                status_ = WRONG_MODE;
                return;
            }
            mode_ = modeMap_[modeName];
        } else if (startsWith(param, stepsKey)) {
            std::string stepsCount = param.substr(stepsKey.length());
            try {
                stepsCount_ = std::stol(stepsCount);
            } catch (std::invalid_argument &e) {
                status_ = WRONG_STEPS;
                return;
            }
        } else if (startsWith(param, configsKey)) {
            std::string fileName = param.substr(configsKey.length());
            if(!setConfigs(fileName)) {
                return;
            }
        } else if (startsWith(param, matrixKey)) {
            std::string fileName = param.substr(configsKey.length());
            if(!setScoreMap(fileName)) {
                return;
            }
        } else {
            names.push_back(param);
        }
    }
    setStrategies(names);
}

void Runner::setMode(TMode mode) {
    mode_ = mode;
}

bool Runner::setStrategies(const std::vector<std::string> &names) {
    assert(!scoreMap_.empty());
    strategies_.clear();
    for (size_t i = 0; i < names.size(); i++) {
        Strategy *strategy = Factory<Strategy, std::string, size_t, TChoiceMatrix &,
                TScoreMap &, TConfigs &>::getInstance()->createProduct(
                names[i], i, choiceMatrix_, scoreMap_, configs_);
        if (nullptr == strategy) {
            status_ = WRONG_STRATEGY_NAME;
            return false;
        }
        names_.push_back(names[i]);
        strategies_[names[i]] = std::unique_ptr<Strategy>(strategy);
    }
    strategiesCount_ = names_.size();
    if (WRONG_STRATEGY_NAME == status_) {
        status_ = OK;
    }
    checkStatus();
    return true;
}

std::map<std::string, size_t> Runner::getGameScores() {
    return gameScores_;
}

void Runner::setStepsCount(size_t stepsCount) {
    stepsCount_ = stepsCount;
}

void Runner::disablePrinting() {
    printMode_ = false;
}

bool Runner::setScoreMap(const std::string &fileName) {
    std::ifstream matrixFile = std::ifstream(defaultMatrixFileName);
    if (!matrixFile.is_open()) {
        status_ = MATRIX_FILE_NOT_OPENED;
        return false;
    }
    if (!parseMatrix(matrixFile)) {
        status_ = WRONG_MATRIX;
        return false;
    }
    return true;
}

bool Runner::setConfigs(const std::string &fileName) {
    std::ifstream configsFile = std::ifstream(fileName);
    if (!configsFile.is_open()) {
        status_ = CONFIGS_FILE_NOT_OPENED;
        return false;
    }
    while (true) {
        std::string word;
        configsFile >> word;
        if (word.empty()) {
            break;
        }
        configs_.push_back(word);
    }
    return true;
}

bool Runner::parseMatrix(std::ifstream &matrixFile) {
    for (size_t i = 0; i < combinationsCount; i++) {
        std::array<TChoice, combLen> combination = {};
        for (size_t j = 0; j < combLen; j++) {
            std::string word;
            matrixFile >> word;
            if (word == "C" || word == "С") {
                combination[j] = COOP;
            } else if (word == "D") {
                combination[j] = DEF;
            } else {
                return false;
            }
        }
        std::array<size_t, combLen> scores = {};
        for (size_t j = 0; j < combLen; j++) {
            std::string word;
            matrixFile >> word;
            try {
                scores[j] = std::stol(word);
            } catch (std::invalid_argument &e) {
                return false;
            }
        }
        scoreMap_[combination] = scores;
    }
    return true;
}

TStatus Runner::getStatus() {
    return status_;
}

bool Runner::runTournament(std::ostream &stream) {
    assert(mode_ == TOURNAMENT);
    if (names_.size() < 4) {
        status_ = NOT_ENOUGH_STRATS;
        return false;
    }
    std::map<std::string, size_t> results;
    for (size_t i = 0; i < strategiesCount_; i++) {
        for (size_t j = i + 1; j < strategiesCount_; j++) {
            for (size_t k = j + 1; k < strategiesCount_; k++) {
                gameScores_ = {};
                for (size_t step = 0; step < stepsCount_; step++) {
                    std::array<TChoice, combLen> choices = {};
                    choices[0] = strategies_[names_[i]]->getChoice();
                    choices[1] = strategies_[names_[j]]->getChoice();
                    choices[2] = strategies_[names_[k]]->getChoice();
                    choiceMatrix_.push_back(choices);
                    std::array<size_t, combLen> scores = scoreMap_[choices];
                    gameScores_[names_[i]] += scores[0];
                    gameScores_[names_[j]] += scores[1];
                    gameScores_[names_[k]] += scores[2];
                }
                printGameResults(stream);
                results[names_[i]] += gameScores_[names_[i]];
                results[names_[j]] += gameScores_[names_[j]];
                results[names_[k]] += gameScores_[names_[k]];
                choiceMatrix_.clear();
                gameScores_.clear();
            }
        }
    }
    gameScores_ = results;
    printTotalResults(stream);
    return true;
}

void Runner::checkStatus() {
    if (mode_ == TOURNAMENT) {
        if (names_.size() < 4) {
            status_ = NOT_ENOUGH_STRATS;
        } else {
            status_ = OK;
        }
    } else {
        if (names_.size() == 3) {
            status_ = OK;
        } else {
            if (names_.size() > 3) {
                status_ = TOO_MANY_STRATS;
            } else {
                status_ = NOT_ENOUGH_STRATS;
            }
        }
    }
}

bool Runner::runGame(std::ostream &stream) {
    if (OK != getStatus()) {
        return false;
    }
    if (stream.fail()) {
        return false;
    }
    if (mode_ == TOURNAMENT) {
        return runTournament(stream);
    }
    if (names_.size() != 3) {
        if (names_.size() > 3) {
            status_ = TOO_MANY_STRATS;
        } else {
            status_ = NOT_ENOUGH_STRATS;
        }
        return false;
    }
    size_t stepsCount = 0;
    while (true) {
        if (mode_ == DETAILED) {
            std::string command;
            std::cin >> command;
            if (quitCommand == command) {
                break;
            }
        }
        stepsCount++;
        std::array<TChoice, combLen> choices = {};
        size_t ind = 0;
        for (const auto &name: names_) {
            choices[ind] = strategies_[name]->getChoice();
            ind++;
        }
        choiceMatrix_.push_back(choices);
        std::array<size_t, combLen> scores = scoreMap_[choices];
        gameScores_[names_[0]] += scores[0];
        gameScores_[names_[1]] += scores[1];
        gameScores_[names_[2]] += scores[2];
        if (mode_ == DETAILED) {
            printStepResults(stream, scores, stepsCount);
        }
        if (mode_ == FAST && stepsCount == stepsCount_) {
            printGameResults(stream);
            break;
        }
    }
    choiceMatrix_.clear();
    return true;
}

void Runner::printStepResults(std::ostream &stream, std::array<size_t, combLen> results, size_t stepNumber) {
    if (!printMode_) {
        return;
    }
    std::map<TChoice, std::string> choiceMap;
    choiceMap[COOP] = "C";
    choiceMap[DEF] = "D";
    stream << "=================== ROUND №" <<
           stepNumber << " ==============" << std::endl;
    stream << "    NAMES    |";
    for (const auto &name: names_) {
        stream << "\t" << name;
    }
    stream << std::endl;
    stream << "   CHOICES   |";
    for (size_t i = 0; i < combLen; i++) {
        stream << "\t" << choiceMap[choiceMatrix_[choiceMatrix_.size() - 1][i]];
    }
    stream << std::endl;
    stream << "ROUND SCORES |";
    for (size_t i = 0; i < combLen; i++) {
        stream << "\t" << results[i];
    }
    stream << std::endl;
    stream << "TOTAL SCORES |";
    for (const auto &name: names_) {
        stream << "\t" << gameScores_[name];
    }
    stream << std::endl;
    stream << "===========================================" << std::endl;
}

void Runner::printGameResults(std::ostream &stream) {
    if (!printMode_) {
        return;
    }
    stream << "=================== GAME RESULTS ==============" << std::endl;
    stream << "STEPS COUNT" << "\t|\t" << stepsCount_ << std::endl;
    stream << "-----------------------------------------------" << std::endl;
    for (const auto &name: names_) {
        if (gameScores_.find(name) == gameScores_.end()) {
            continue;
        }
        stream << name << "\t|\t" << gameScores_[name] << std::endl;
    }
    stream << "===============================================" << std::endl;
}

void Runner::printTotalResults(std::ostream &stream) {
    if (!printMode_) {
        return;
    }
    stream << "================== TOTAL RESULTS ==============" << std::endl;
    size_t maxScore = 0;
    for (const auto &name: names_) {
        if (gameScores_[name] > maxScore) {
            maxScore = gameScores_[name];
        }
    }
    for (const auto &name: names_) {
        if (gameScores_[name] == maxScore) {
            stream << name << "\t|\t" << gameScores_[name]
                   << "\t\t<--- WINNER" << std::endl;
        } else {
            stream << name << "\t|\t" << gameScores_[name] << std::endl;
        }
    }
    stream << "==============================================" << std::endl;
}

void Runner::printErrorMessage(std::ostream &stream) {
    if (status_ == OK) {
        return;
    }
    if (errorMessages_.find(status_) == errorMessages_.end()) {
        stream << "Unknown error" << std::endl;
    } else {
        stream << errorMessages_[status_] << std::endl;
    }
}
