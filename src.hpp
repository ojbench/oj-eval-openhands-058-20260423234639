
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <stdexcept>

class BasicException {
protected:
    const char *message;

public:
    explicit BasicException(const char *_message) : message(_message) {}

    virtual const char *what() const {
        return message;
    }
};

class ArgumentException: public BasicException {
public:
    explicit ArgumentException(const char *_message) : BasicException(_message) {}
};

class IteratorException: public BasicException {
public:
    explicit IteratorException(const char *_message) : BasicException(_message) {}
};

struct Pokemon {
    char name[12];
    int id;
    std::vector<std::string> types;
    
    Pokemon() : id(0) {
        memset(name, 0, sizeof(name));
    }
    
    Pokemon(const char *_name, int _id, const std::vector<std::string>& _types) : id(_id), types(_types) {
        strncpy(name, _name, 11);
        name[11] = '\0';
    }
};

class Pokedex {
private:
    std::vector<Pokemon> pokemons;
    std::string fileName;
    
    static const std::vector<std::string> VALID_TYPES;
    
    bool isValidName(const char *name) const {
        if (!name || strlen(name) == 0 || strlen(name) > 10) return false;
        for (size_t i = 0; i < strlen(name); i++) {
            if (!isalpha(name[i])) return false;
        }
        return true;
    }
    
    bool isValidType(const std::string &type) const {
        return std::find(VALID_TYPES.begin(), VALID_TYPES.end(), type) != VALID_TYPES.end();
    }
    
    std::vector<std::string> parseTypes(const char *types) const {
        std::vector<std::string> result;
        std::string str(types);
        std::stringstream ss(str);
        std::string token;
        
        while (std::getline(ss, token, '#')) {
            result.push_back(token);
        }
        return result;
    }
    
    int findPokemonIndex(int id) const {
        for (size_t i = 0; i < pokemons.size(); i++) {
            if (pokemons[i].id == id) return i;
        }
        return -1;
    }
    
    float getDamageMultiplier(const std::string &attackType, const std::string &defendType) const {
        if (attackType == "normal") {
            return 1.0f;
        }
        
        if (attackType == "fire") {
            if (defendType == "grass") return 2.0f;
            if (defendType == "ice") return 2.0f;
            if (defendType == "water") return 0.5f;
            if (defendType == "fire") return 0.5f;
        }
        
        if (attackType == "water") {
            if (defendType == "fire") return 2.0f;
            if (defendType == "ground") return 2.0f;
            if (defendType == "water") return 0.5f;
            if (defendType == "grass") return 0.5f;
        }
        
        if (attackType == "grass") {
            if (defendType == "water") return 2.0f;
            if (defendType == "ground") return 2.0f;
            if (defendType == "fire") return 0.5f;
            if (defendType == "grass") return 0.5f;
            if (defendType == "flying") return 0.5f;
        }
        
        if (attackType == "electric") {
            if (defendType == "water") return 2.0f;
            if (defendType == "flying") return 2.0f;
            if (defendType == "grass") return 0.5f;
            if (defendType == "electric") return 0.5f;
            if (defendType == "ground") return 0.0f;
        }
        
        if (attackType == "ice") {
            if (defendType == "grass") return 2.0f;
            if (defendType == "ground") return 2.0f;
            if (defendType == "flying") return 2.0f;
            if (defendType == "fire") return 0.5f;
            if (defendType == "water") return 0.5f;
            if (defendType == "ice") return 0.5f;
        }
        
        if (attackType == "flying") {
            if (defendType == "grass") return 2.0f;
            if (defendType == "ground") return 2.0f;
            if (defendType == "electric") return 0.5f;
        }
        
        return 1.0f;
    }
    
    void sortPokemons() {
        std::sort(pokemons.begin(), pokemons.end(), 
                  [](const Pokemon &a, const Pokemon &b) { return a.id < b.id; });
    }

public:
    explicit Pokedex(const char *_fileName) : fileName(_fileName) {
        std::ifstream file(fileName);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line.empty()) continue;
                
                std::stringstream ss(line);
                std::string name, typesStr;
                int id;
                
                if (std::getline(ss, name, ',') && ss >> id && ss.ignore() && std::getline(ss, typesStr)) {
                    std::vector<std::string> types = parseTypes(typesStr.c_str());
                    pokemons.emplace_back(name.c_str(), id, types);
                }
            }
            file.close();
            sortPokemons();
        }
    }

    ~Pokedex() {
        std::ofstream file(fileName);
        if (file.is_open()) {
            for (const auto &pokemon : pokemons) {
                file << pokemon.name << "," << pokemon.id << ",";
                for (size_t i = 0; i < pokemon.types.size(); i++) {
                    if (i > 0) file << "#";
                    file << pokemon.types[i];
                }
                file << "\n";
            }
            file.close();
        }
    }

    bool pokeAdd(const char *name, int id, const char *types) {
        if (!isValidName(name)) {
            throw ArgumentException(("Argument Error: PM Name Invalid (" + std::string(name) + ")").c_str());
        }
        
        if (id <= 0) {
            throw ArgumentException(("Argument Error: PM ID Invalid (" + std::to_string(id) + ")").c_str());
        }
        
        std::vector<std::string> typeList = parseTypes(types);
        for (const auto &type : typeList) {
            if (!isValidType(type)) {
                throw ArgumentException(("Argument Error: PM Type Invalid (" + type + ")").c_str());
            }
        }
        
        if (typeList.empty() || typeList.size() > 7) {
            throw ArgumentException("Argument Error: PM Type Invalid (empty or too many types)");
        }
        
        if (findPokemonIndex(id) != -1) {
            return false;
        }
        
        pokemons.emplace_back(name, id, typeList);
        sortPokemons();
        return true;
    }

    bool pokeDel(int id) {
        int index = findPokemonIndex(id);
        if (index == -1) {
            return false;
        }
        
        pokemons.erase(pokemons.begin() + index);
        return true;
    }

    std::string pokeFind(int id) const {
        int index = findPokemonIndex(id);
        if (index == -1) {
            return "None";
        }
        
        return std::string(pokemons[index].name);
    }

    std::string typeFind(const char *types) const {
        std::vector<std::string> searchTypes = parseTypes(types);
        
        for (const auto &type : searchTypes) {
            if (!isValidType(type)) {
                throw ArgumentException(("Argument Error: PM Type Invalid (" + type + ")").c_str());
            }
        }
        
        std::vector<Pokemon> results;
        
        for (const auto &pokemon : pokemons) {
            bool hasAllTypes = true;
            for (const auto &searchType : searchTypes) {
                bool found = false;
                for (const auto &pokemonType : pokemon.types) {
                    if (pokemonType == searchType) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    hasAllTypes = false;
                    break;
                }
            }
            if (hasAllTypes) {
                results.push_back(pokemon);
            }
        }
        
        if (results.empty()) {
            return "None";
        }
        
        std::sort(results.begin(), results.end(), 
                  [](const Pokemon &a, const Pokemon &b) { return a.id < b.id; });
        
        std::string result = std::to_string(results.size());
        for (const auto &pokemon : results) {
            result += "\n" + std::string(pokemon.name);
        }
        
        return result;
    }

    float attack(const char *type, int id) const {
        int index = findPokemonIndex(id);
        if (index == -1) {
            return -1.0f;
        }
        
        if (!isValidType(type)) {
            throw ArgumentException(("Argument Error: PM Type Invalid (" + std::string(type) + ")").c_str());
        }
        
        const Pokemon &defender = pokemons[index];
        float totalDamage = 1.0f;
        
        for (const auto &defendType : defender.types) {
            totalDamage *= getDamageMultiplier(type, defendType);
        }
        
        return totalDamage;
    }

    int catchTry() const {
        if (pokemons.empty()) {
            return 0;
        }
        
        std::vector<Pokemon> owned = {pokemons[0]};
        std::vector<bool> captured(pokemons.size(), false);
        captured[0] = true;
        
        bool newCapture = true;
        while (newCapture) {
            newCapture = false;
            
            for (size_t i = 0; i < pokemons.size(); i++) {
                if (captured[i]) continue;
                
                const Pokemon &wild = pokemons[i];
                
                for (const Pokemon &trainer : owned) {
                    bool canCapture = false;
                    
                    for (const auto &attackType : trainer.types) {
                        float damage = 1.0f;
                        for (const auto &defendType : wild.types) {
                            damage *= getDamageMultiplier(attackType, defendType);
                        }
                        
                        if (damage >= 2.0f) {
                            canCapture = true;
                            break;
                        }
                    }
                    
                    if (canCapture) {
                        owned.push_back(wild);
                        captured[i] = true;
                        newCapture = true;
                        break;
                    }
                }
            }
        }
        
        return owned.size();
    }

    struct iterator {
    private:
        Pokedex *pokedex;
        size_t index;
        
    public:
        iterator(Pokedex *_pokedex, size_t _index) : pokedex(_pokedex), index(_index) {}
        
        iterator &operator++() {
            if (index < pokedex->pokemons.size()) {
                index++;
            } else {
                throw IteratorException("Iterator Error: Increment beyond end");
            }
            return *this;
        }
        
        iterator &operator--() {
            if (index > 0) {
                index--;
            } else {
                throw IteratorException("Iterator Error: Decrement before begin");
            }
            return *this;
        }
        
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }
        
        iterator & operator = (const iterator &rhs) {
            pokedex = rhs.pokedex;
            index = rhs.index;
            return *this;
        }
        
        bool operator == (const iterator &rhs) const {
            return pokedex == rhs.pokedex && index == rhs.index;
        }
        
        bool operator != (const iterator &rhs) const {
            return !(*this == rhs);
        }
        
        Pokemon & operator*() const {
            if (index >= pokedex->pokemons.size()) {
                throw IteratorException("Iterator Error: Dereference invalid iterator");
            }
            return const_cast<Pokemon&>(pokedex->pokemons[index]);
        }
        
        Pokemon *operator->() const {
            if (index >= pokedex->pokemons.size()) {
                throw IteratorException("Iterator Error: Dereference invalid iterator");
            }
            return const_cast<Pokemon*>(&pokedex->pokemons[index]);
        }
    };

    iterator begin() {
        return iterator(this, 0);
    }

    iterator end() {
        return iterator(this, pokemons.size());
    }
};

const std::vector<std::string> Pokedex::VALID_TYPES = {
    "normal", "fire", "water", "grass", "electric", "ice", "flying"
};

