#pragma once
#include <bitset>
#include <array>
#include <allocators>

template<class Type>
class poolAllocBlock {

    //using type = Type;
    //using blockSize = blockSize;
    //using Type = int;
    static const size_t blockSize = 4096u / 4;//#TODO force to be multiple of 8
    using Pointer = Type*;
    using freelistType = std::bitset<blockSize + blockSize % (blockSize <= 32 ? 32 : 64)>;
public:
    //#TODO optimize for page size (4096)
    freelistType freelist;//#TODO subclass and add methods to find first set bit
    std::array<char[sizeof(Type)], blockSize> data;
    bool hasFree = true;

    poolAllocBlock() noexcept {
        freelist.set(); //all is free
        //except placeholders
        for (auto i = blockSize; i < blockSize + blockSize % (sizeof(freelistType::_Ty)*8); i++) {
            freelist.set(i, false);
        }
    }

    bool freeListFastHasFreeCheck() const {
        auto maxI = (blockSize + blockSize % (sizeof(freelistType::_Ty)*8)) / (sizeof(freelistType::_Ty)*8);
        for (auto i = 0u; i < maxI; i++) {
            auto val = freelist._Getword(i);
            if (freelist._Getword(i)) return true;
        }
        return false;
    }



    __declspec(noinline) Pointer allocate(const std::size_t count) throw (std::bad_alloc) {	// allocate array of _Count elements
        if (count != 1)throw std::bad_alloc();
        for (auto i = 0u; i < blockSize; i++) {
            if (freelist.test(i)) {
                freelist.set(i, false);
                hasFree = freeListFastHasFreeCheck();
                return reinterpret_cast<Pointer>(&data[i]);
            }
        }
        throw std::bad_alloc();
        return nullptr;
    }

    void deallocate(const Pointer ptr) {
        if (!isInBounds(ptr)) throw std::bad_alloc(); //actually bad dealloc
        size_t index = (reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(data.data())) / sizeof(Type);
        freelist.set(index, true);//deallocated now
        hasFree = true;
    }
    void deallocate(const Pointer ptr, const std::size_t count) {
        if (count != 1)throw std::bad_alloc();//actually bad dealloc
        deallocate(ptr);
    }
    __declspec(noinline) bool hasFreeElements() const noexcept {
        return hasFree;
    }
    std::pair<uintptr_t, uintptr_t> getBounds() {
        return { reinterpret_cast<uintptr_t>(data.data()), reinterpret_cast<uintptr_t>(data.data()) + blockSize };
    }
    bool isInBounds(const Pointer ptr) {
        auto bounds = getBounds();
        return bounds.first <= reinterpret_cast<uintptr_t>(ptr) && bounds.second >= reinterpret_cast<uintptr_t>(ptr);
    }
};

template<class Type>
class poolAllocator {
    using Pointer = Type*;
    using BlockType = poolAllocBlock<Type>;


    std::vector<std::unique_ptr<BlockType>> blocks; //#TODO keep freelist of blocks that have atleast 1 free element

public:


    Pointer allocate(const std::size_t count) throw (std::bad_alloc) {	// allocate array of _Count elements
        if (count != 1) throw std::bad_alloc();
        for (auto& block : blocks) {
            if (block->hasFreeElements())
                return block->allocate(count);
        }
        //allocate new block
        auto newBlock = blocks.emplace(blocks.end(), std::make_unique<BlockType>());
        return (*newBlock)->allocate(count);
    }

    void deallocate(const Pointer ptr) {
        for (auto& block : blocks) {
            if (block->isInBounds(ptr))
                return block->deallocate(ptr);
        }
    }
    void deallocate(const Pointer ptr, const std::size_t) {
        deallocate(ptr);
    }
};



