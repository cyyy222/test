#include "Dictionary.h"
#include "Configuration.h"

#include <iostream>
#include <sstream>
#include <fstream>

using std::cout;
using std::endl;
using std::ifstream;
using std::istringstream;

namespace wdcpp
{
Dictionary *Dictionary::_singletonDict = getInstance(); //饿汉模式
Dictionary *Dictionary::getInstance()
{
    if (_singletonDict == nullptr)
    {
        _singletonDict = new Dictionary();
        _singletonDict->initDict();
        _singletonDict->initIndex();
        atexit(destory);
    }
    return _singletonDict;
}

void Dictionary::destory()
{
    if (_singletonDict)
    {
        delete _singletonDict;
        _singletonDict = nullptr;
    }
}

void Dictionary::initDict()
{
    cout << "初始化词典" << endl;
    string dictPath = Configuration::getInstance()->getConfigMap()["dict"];

    ifstream ifs(dictPath);
    if (!ifs.good())
    {
        cout << "打开" << string(dictPath) << " 文件错误 !" << endl;
        return;
    }
    string line;
    while (getline(ifs, line))
    {
        istringstream iss(line);
        string word, freq;
        iss >> word >> freq;
        _dict.push_back(pair<string, int>(word, freq == "" ? 0 : (stoi(freq))));
    }
}



void Dictionary::initIndex() // key : set<id>
{
    cout << "初始化索引" << endl;
    string dictIndexPath = Configuration::getInstance()->getConfigMap()["dicIndex"];
    // string enDictIndexPath = Configuration::getInstance()->getConfigMap()["enDicIndex"];

    ifstream ifs(dictIndexPath);
    if (!ifs.good())
    {
        cout << "打开" << string(dictIndexPath) << " 文件错误 !" << endl;
        return;
    }
    string line;
    while (getline(ifs, line))
    {
        istringstream iss(line);
        string word, locate;
        iss >> word;
        while (iss >> locate)
        {
            // _index[word].insert(stoi(locate));
            _index[word].insert(locate == "" ? 0 : stoi(locate));
        }
    }
}



const vector<pair<string, int>> &Dictionary::getDict()
{
    return _dict;
}

const map<string, set<int>> &Dictionary::getIndexTable()
{
    return _index;
}

};
