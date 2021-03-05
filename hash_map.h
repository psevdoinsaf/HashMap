#include <vector>
#include <list>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <iostream>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
private:
    Hash hasher;
    std::vector<std::list<std::pair<const KeyType, ValueType>>> data;
    size_t data_size = 0;

public:
    HashMap(Hash new_hasher = Hash()) : hasher(new_hasher) {
        if (data.empty()) {
            data.resize(1);
        }
    }

    template<typename iterator>
    HashMap(iterator begin, iterator end, Hash new_hasher = Hash()) : hasher(new_hasher) {
        if (data.empty()) {
            data.resize(1);
        }
        for (auto i = begin; i != end; ++i) {
            insert(*i);
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list, Hash new_hasher = Hash()) : hasher(new_hasher) {
        if (data.empty()) {
            data.resize(1);
        }
        for (auto i : list) {
            insert(i);
        }
    }

    size_t size() const {
        return data_size;
    }

    bool empty() const {
        return (data_size == 0);
    }

    Hash hash_function() const {
        return hasher;
    }

    void insert(std::pair<const KeyType, ValueType> value) {
        if (data.empty()) {
            data.resize(1);
        }
        size_t position = hasher(value.first) % (data.size());
        for (auto &i : data[position]) {
            if (i.first == value.first) {
                return;
            }
        }
        ++data_size;
        data[position].push_back(value);
        if (data_size > data.size()) {
            reallocate(data_size * 2);
        }
    }

    void erase(KeyType key) {
        if (data.empty()) {
            data.resize(1);
            return;
        }
        size_t position = hasher(key) % (data.size());
        auto it = data[position].begin();
        for (auto &i : data[position]) {
            if (i.first == key) {
                data[position].erase(it);
                --data_size;
                if (data_size * 4 <= data.size()) {
                    reallocate(data_size * 2);
                }
                return;
            }
            ++it;
        }
    }


    class iterator {
    private:
        size_t bucket_id;
        typename std::list<std::pair<const KeyType, ValueType>>::iterator element_iterator;
        std::vector<std::list<std::pair<const KeyType, ValueType>>> *data_iterator;

    public:
        iterator() = default;

        iterator(size_t new_bucket_id,
                 typename std::list<std::pair<const KeyType, ValueType>>::iterator new_element_iterator,
                 std::vector<std::list<std::pair<const KeyType, ValueType>>> *new_data_iterator) : bucket_id(
                new_bucket_id), element_iterator(new_element_iterator), data_iterator(new_data_iterator) {
        }

        iterator(const iterator &other) {
            bucket_id = other.bucket_id;
            element_iterator = other.element_iterator;
            data_iterator = other.data_iterator;
        }

        iterator &operator++() {
            ++element_iterator;
            if (element_iterator != (*data_iterator)[bucket_id].end()) {
                return *this;
            }
            for (size_t bucket = bucket_id + 1; bucket < data_iterator->size(); ++bucket) {
                if (!(*data_iterator)[bucket].empty()) {
                    bucket_id = bucket;
                    element_iterator = (*data_iterator)[bucket].begin();
                    return *this;
                }
            }
            bucket_id = (*data_iterator).size() - 1;
            element_iterator = ((*data_iterator)[(*data_iterator).size() - 1]).end();
            return *this;
        }

        iterator operator++(int) {
            iterator was = iterator(*this);
            ++element_iterator;
            if (element_iterator != (*data_iterator)[bucket_id].end()) {
                return was;
            }
            for (size_t bucket = bucket_id + 1; bucket < data_iterator->size(); ++bucket) {
                if (!(*data_iterator)[bucket].empty()) {
                    bucket_id = bucket;
                    element_iterator = (*data_iterator)[bucket].begin();
                    return was;
                }
            }
            bucket_id = (*data_iterator).size() - 1;
            element_iterator = ((*data_iterator)[(*data_iterator).size() - 1]).end();
            return was;
        }

        std::pair<const KeyType, ValueType> &operator*() {
            return (*element_iterator);
        }

        typename std::list<std::pair<const KeyType, ValueType>>::iterator &operator->() {
            return element_iterator;
        }

        bool operator==(const iterator &other) const {
            return (bucket_id == other.bucket_id && element_iterator == other.element_iterator &&
                    data_iterator == other.data_iterator);
        }

        bool operator!=(const iterator &other) const {
            return (bucket_id != other.bucket_id || element_iterator != other.element_iterator ||
                    data_iterator != other.data_iterator);
        }
    };

    iterator begin() {
        if (data.empty()) {
            data.resize(1);
        }
        for (size_t bucket = 0; bucket < data.size(); ++bucket) {
            if (!data[bucket].empty()) {
                iterator _begin = iterator(bucket, data[bucket].begin(), &data);
                return _begin;
            }
        }
        iterator _end = iterator(data.size() - 1, data[data.size() - 1].end(), &data);
        return _end;
    }

    iterator end() {
        if (data.empty()) {
            data.resize(1);
        }
        iterator _end(data.size() - 1, data[data.size() - 1].end(), &data);
        return _end;
    }

    class const_iterator {
    private:
        size_t bucket_id;
        typename std::list<std::pair<const KeyType, ValueType>>::const_iterator element_iterator;
        const std::vector<std::list<std::pair<const KeyType, ValueType>>> *data_iterator;

    public:
        const_iterator() = default;


        const_iterator(size_t new_bucket_id,
                       typename std::list<std::pair<const KeyType, ValueType>>::const_iterator new_element_iterator,
                       const std::vector<std::list<std::pair<const KeyType, ValueType>>> *new_data_iterator)
                : bucket_id(new_bucket_id), element_iterator(new_element_iterator), data_iterator(new_data_iterator) {}

        const_iterator(const const_iterator &other) {
            bucket_id = other.bucket_id;
            element_iterator = other.element_iterator;
            data_iterator = other.data_iterator;
        }

        const_iterator(iterator &other) {
            bucket_id = other.bucket_id;
            element_iterator = other.element_iterator;
            data_iterator = other.data_iterator;
        }

        const_iterator &operator++() {
            ++element_iterator;
            if (element_iterator != (*data_iterator)[bucket_id].end()) {
                return *this;
            }
            for (size_t bucket = bucket_id + 1; bucket < data_iterator->size(); ++bucket) {
                if (!(*data_iterator)[bucket].empty()) {
                    bucket_id = bucket;
                    element_iterator = data_iterator->at(bucket).begin();
                    return *this;
                }
            }
            bucket_id = (*data_iterator).size() - 1;
            element_iterator = ((*data_iterator)[(*data_iterator).size() - 1]).end();
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator was = const_iterator(*this);
            ++element_iterator;
            if (element_iterator != (*data_iterator)[bucket_id].end()) {
                return was;
            }
            for (size_t bucket = bucket_id + 1; bucket < data_iterator->size(); ++bucket) {
                if (!(*data_iterator)[bucket].empty()) {
                    bucket_id = bucket;
                    element_iterator = (*data_iterator)[bucket].begin();
                    return was;
                }
            }
            bucket_id = (*data_iterator).size() - 1;
            element_iterator = ((*data_iterator)[(*data_iterator).size() - 1]).end();
            return was;
        }

        const std::pair<const KeyType, ValueType> &operator*() const {
            return (*element_iterator);
        }

        const typename std::list<std::pair<const KeyType, ValueType>>::const_iterator &operator->() const {
            return element_iterator;
        }

        bool operator==(const const_iterator &other) const {
            return (bucket_id == other.bucket_id && element_iterator == other.element_iterator &&
                    data_iterator == other.data_iterator);
        }

        bool operator!=(const const_iterator &other) const {
            return (bucket_id != other.bucket_id || element_iterator != other.element_iterator ||
                    data_iterator != other.data_iterator);
        }
    };

    const_iterator begin() const {
        for (size_t bucket = 0; bucket < data.size(); ++bucket) {
            if (!data.at(bucket).empty()) {
                const_iterator _begin = const_iterator(bucket, data.at(bucket).begin(), &data);
                return _begin;
            }
        }
        const_iterator _end = const_iterator(data.size() - 1, data[data.size() - 1].end(), &data);
        return _end;
    }

    const_iterator end() const {

        const_iterator _end = const_iterator(data.size() - 1, data[data.size() - 1].end(), &data);
        return _end;
    }

    iterator find(KeyType Key) {
        size_t position = hasher(Key) % data.size();
        for (auto i = data[position].begin(); i != data[position].end(); ++i) {
            if (data[position].empty())
                break;
            if ((*i).first == Key) {
                iterator it = iterator(position, i, &data);
                return it;
            }
        }
        return end();
    }

    const_iterator find(KeyType Key) const {
        size_t position = hasher(Key) % data.size();
        for (auto i = data[position].begin(); i != data[position].end(); ++i) {
            if (data[position].empty())
                break;
            if (i->first == Key) {
                const_iterator it = const_iterator(position, i, &data);
                return it;
            }
        }
        return end();
    }

    ValueType &operator[](KeyType Key) {
        auto it = find(Key);
        if (it != end()) {
            return it->second;
        }
        insert({Key, ValueType()});
        return find(Key)->second;
    }

    const ValueType &at(KeyType Key) const {
        const_iterator it = find(Key);
        if (it != end()) {
            return it->second;
        }
        throw std::out_of_range("there is no such element here");
    }

    void clear() {
        data_size = 0;
        data.clear();
        data.resize(1);
    }

    void reallocate(const size_t reallocation_size) { //TODO
        if (reallocation_size < 1)
            return;
        std::vector<std::list<std::pair<const KeyType, ValueType>>> old_data = data;
        data.clear();
        data.resize(reallocation_size);
        data_size = 0;
        for (auto i : old_data) {
            for (auto j : i) {
                insert(j);
            }
        }
    }

    HashMap &operator=(HashMap &other) {
        if (this != &other) {
            (*this).clear();
            hasher = other.hasher;
            for (size_t bucket = 0; bucket < other.data.size(); ++bucket) {
                for (const auto &it : other.data[bucket]) {
                    insert(it);
                }
            }
        }
        return *this;
    }

    HashMap(HashMap &other) : hasher(other.hasher) {
        (*this).clear();
        for (size_t bucket = 0; bucket < other.data.size(); ++bucket) {
            for (const auto &it : other.data[bucket]) {
                insert(it);
            }
        }
    }
};
