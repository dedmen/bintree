#include <cstdint>
#include <memory>
#include <functional>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stack>
#include <allocators>
#include <vector>
#include "poolAlloc.h"
#include <thread>
/*
pool-allocator with freelist
binary-heap container
*/


template<class Type, class Alloc>
class bintree;

template<class Type>
class bintreeElement {
public:

    bintreeElement* parent{ nullptr };
    bintreeElement* leftEl{ nullptr };
    bintreeElement* rightEl{ nullptr };
    Type value;

    explicit bintreeElement(bintreeElement* _parent, Type&& v) : parent(_parent), value(std::forward<Type>(v)) {}
    ~bintreeElement() {
        deAlloc(leftEl);
        deAlloc(rightEl);
    }

    explicit operator const Type&() const {
        return value;
    }

    bool hasLeft() const noexcept { return leftEl != nullptr; }
    bool hasRight() const noexcept { return rightEl != nullptr; }


    const Type& getLeftElement() const {
        return *leftEl;
    }

    const Type& getRightElement() const {
        return *rightEl;
    }

    void insertElement(bintreeElement* el) {
        auto me = this;
        while (true) {
            if (el->value < me->value && !me->leftEl) {
                me->leftEl = el;
                el->parent = me;
                return;
            }
            if (el->value > me->value && !me->rightEl) {
                me->rightEl = el;
                el->parent = me;
                return;
            }
            if (el->value < me->value && me->leftEl) {
                me = me->leftEl;
                continue;
            }
            if (el->value > me->value && me->rightEl) me = me->rightEl;
        }

    }
};

template<class Type, class Alloc = std::allocator<bintreeElement<Type>>>
class bintree {
    using bintreeElement = bintreeElement<Type>;
    Alloc allocator = Alloc();
    bintreeElement* alloc(bintreeElement* par, Type&& initV) {
        auto newElem = allocator.allocate(1);
        ::new(newElem) bintreeElement(par, std::forward<Type>(initV));
        return newElem;
    }

    void deAlloc(bintreeElement* elem) {
        if (elem) allocator.deallocate(elem, 1);
    }

    bintreeElement* root;
    uint32_t elemCount = 0;
public:

    class iterator : public std::iterator<std::bidirectional_iterator_tag, Type> {
        const bintreeElement* me = nullptr;
        const bintreeElement* lastEl = nullptr;
        const bintree* tree;
        enum class StepType {
            norm,
            getRight,
            getLeft
        } curStep = StepType::norm;
    public:
        iterator(const iterator& other) : me(other.me), curStep(other.curStep), tree(other.tree) {}
        explicit iterator(const bintree& bt) { tree = &bt; me = bt.root; getNext(); }
        explicit iterator(const bintree& bt, const bintreeElement* st) { tree = &bt; me = st; }
        //const const_iterator& operator=(const const_iterator& other) { return other; }

        iterator& operator++() {
            getNext();
            return *this;
        } // prefix++
        iterator  operator++(int) {
            iterator tmp(*this);
            getNext();
            return tmp;
        } // postfix++
        iterator& operator--() {
            getPrevious();
            return *this;
        } // prefix--
        iterator  operator--(int) {
            iterator tmp(*this);
            getPrevious();
            return tmp;
        } // postfix--
        void     operator+=(const std::size_t& n) {
            for (size_t i = 0; i < n; ++i) {
                getNext();
            }
        }
        iterator operator+ (const std::size_t& n) const {
            iterator tmp(*this);
            for (size_t i = 0; i < n; ++i) {
                tmp->getNext();
            }
            return tmp;
        }
        void     operator-=(const std::size_t& n) {
            for (size_t i = 0; i < n; ++i) {
                getPrevious();
            }
        }
        iterator operator- (const std::size_t& n) const {
            iterator tmp(*this);
            for (size_t i = 0; i < n; ++i) {
                tmp->getPrevious();
            }
            return tmp;
        }

        //#TODO only implement if Type has these operators
        bool operator< (const iterator& other) const {
            if (!me || !other.me) return false;
            return (me->value < other.me->value);
        }
        bool operator<=(const iterator& other) const {
            if (!me || !other.me) return false;
            return (me->value <= other.me->value);
        }
        bool operator> (const iterator& other) const {
            if (!me || !other.me) return false;
            return (me->value > other.me->value);
        }
        bool operator>=(const iterator& other) const {
            if (!me || !other.me) return false;
            return (me->value >= other.me->value);
        }
        bool operator==(const iterator& other) const {
            if (!me || !other.me) return false;
            return  (me->value == other.me->value);
        }
        bool operator!=(const iterator& other) const {
            if (me == other.me) return false;
            if (!me || !other.me) return true;
            return  (me->value != other.me->value);
        }

        const Type& operator*() { return me->value; }
        Type* operator->() { return  &me->value; }
        explicit operator Type*() { return &me->value; }
        explicit operator Type() const { return me->value; }

        void getNext() {

            //if (curStep == StepType::getLeft) {
            //    if (me->leftEl != nullptr) {
            //        lastEl = me;
            //        me = me->leftEl;
            //    } else {
            //        lastEl = nullptr;
            //    }
            //    if (lastEl == me->leftEl) {
            //        lastEl = me;
            //        me = me->parent;
            //    }
            //}

            if (curStep == StepType::getRight) {
                if (me->rightEl != nullptr) {
                    lastEl = me;
                    me = me->rightEl;
                } else {
                    lastEl = nullptr;
                }
                if (lastEl == me->rightEl) {
                    lastEl = me;
                    me = me->parent;
                }
            }
            curStep = StepType::norm;


            while (me != nullptr) {
                if (lastEl == me->parent) {
                    if (me->leftEl != nullptr) {
                        lastEl = me;
                        me = me->leftEl;
                        continue;
                    } else {
                        lastEl = nullptr;
                    }
                }
                if (lastEl == me->leftEl) {


                    curStep = StepType::getRight;
                    return;

                }
                if (lastEl == me->rightEl) {
                    lastEl = me;
                    me = me->parent;
                }
            }
        }


        void getPrevious() {
            auto meAtStart = me;
            if (curStep == StepType::getLeft) {
                if (me->leftEl != nullptr) {
                    lastEl = me;
                    me = me->leftEl;
                } else {
                    lastEl = nullptr;
                }
                if (lastEl == me->leftEl) {
                    lastEl = me;
                    me = me->parent;
                }
            }
            curStep = StepType::norm;

            while (me != nullptr) {
                if (lastEl == me->parent) {
                    if (me->rightEl != nullptr) {
                        lastEl = me;
                        me = me->rightEl;
                        continue;
                    } else {
                        lastEl = nullptr;
                    }
                }
                if (lastEl == me->rightEl) {
                    if (me->value < meAtStart->value) {
                        curStep = StepType::getLeft;
                        return;
                    }
                    if (me->leftEl != nullptr) {
                        lastEl = me;
                        me = me->leftEl;
                        continue;
                    } else {
                        lastEl = nullptr;
                    }
                }
                if (lastEl == me->leftEl) {
                    lastEl = me;
                    me = me->parent;
                }

            }
        }

    };

public:

    template <typename Func>
    void inOrder(Func func) { //O(N)
        if (!root) return;
        auto me = root;
        bintreeElement* lastEl = nullptr;


        
        while (me != nullptr) {
            if (lastEl == me->parent) {
                if (me->leftEl != nullptr) {
                    lastEl = me;
                    me = me->leftEl;
                    continue;
                } else {
                    lastEl = nullptr;
                }
            }
            if (lastEl == me->leftEl) {
                func(me->value);
                if (me->rightEl != nullptr) {
                    lastEl = me;
                    me = me->rightEl;
                    continue;
                } else {
                    lastEl = nullptr;
                }
            }
            if (lastEl == me->rightEl) {
                lastEl = me;
                me = me->parent;
            }
        }
    }
    template <typename Func>
    void preOrder(Func func) { //O(N)
        if (!root) return;
        auto me = root;
        bintreeElement* lastEl = nullptr;



        while (me != nullptr) {
            if (lastEl == me->parent) {
                func(me->value);
                if (me->leftEl != nullptr) {
                    lastEl = me;
                    me = me->leftEl;
                    continue;
                } else {
                    lastEl = nullptr;
                }
            }
            if (lastEl == me->leftEl) {
                if (me->rightEl != nullptr) {
                    lastEl = me;
                    me = me->rightEl;
                    continue;
                } else {
                    lastEl = nullptr;
                }
            }
            if (lastEl == me->rightEl) {
                lastEl = me;
                me = me->parent;
            }
        }
    }
    template <typename Func>
    void postOrder(Func func) { //O(N)
        if (!root) return;

        auto me = root;
        auto last = root;
        bintreeElement* lastEl = nullptr;
        //while (me->leftEl) {//Go to leftmost element
        //    me = me->leftEl;
        //}


        while (me != nullptr) {
            if (lastEl == me->parent) {
                if (me->leftEl != nullptr) {
                    lastEl = me;
                    me = me->leftEl;
                    continue;
                } else {
                    lastEl = nullptr;
                }
            }
            if (lastEl == me->leftEl) {
                if (me->rightEl != nullptr) {
                    lastEl = me;
                    me = me->rightEl;
                    continue;
                } else {
                    lastEl = nullptr;
                }
            }
            if (lastEl == me->rightEl) {
                lastEl = me;
                func(me->value);
                me = me->parent;
            }
        }
    }

    template <typename Func>
    void inOrderBackwards(Func func) { //O(N)
        if (!root) return;
        auto me = root;
        bintreeElement* lastEl = nullptr;
        while (me != nullptr) {
            if (lastEl == me->parent) {
                if (me->rightEl != nullptr) {
                    lastEl = me;
                    me = me->rightEl;
                    continue;
                } else {
                    lastEl = nullptr;
                }
            }
            if (lastEl == me->rightEl) {
                func(me->value);
                if (me->leftEl != nullptr) {
                    lastEl = me;
                    me = me->leftEl;
                    continue;
                } else {
                    lastEl = nullptr;
                }
            }
            if (lastEl == me->leftEl) {
                lastEl = me;
                me = me->parent;
            }

        }
    }

    size_t depth() {//O(N) visits every element
        if (!root) return 0;
        size_t maxLevel = 0;
        size_t curLevel = 1;
        //just a copy of inOrder...
        const bintreeElement* me = root;
        const bintreeElement* lastEl = nullptr;
        while (me != nullptr) {
            if (lastEl == me->parent) {
                if (me->leftEl != nullptr) {
                    lastEl = me;
                    me = me->leftEl;
                    curLevel++;
                    continue;
                } else {
                    lastEl = nullptr;
                }
            }
            if (lastEl == me->leftEl) {
                maxLevel = std::max(curLevel, maxLevel);
                if (me->rightEl != nullptr) {
                    lastEl = me;
                    me = me->rightEl;
                    curLevel++;
                    continue;
                } else {
                    lastEl = nullptr;
                }
            }
            if (lastEl == me->rightEl) {
                lastEl = me;
                me = me->parent;
                curLevel--;
            }
        }
        return maxLevel;
    }

    const Type& emplace(const iterator& hint, Type&& elem) {//O(N) on empty tree or worst case. O(log2 N) on balanced tree
        ++elemCount; //#TODO we may reject duplicates
        if (!root) {
            root = alloc(nullptr, std::forward<Type>(elem));
            return root->value;
        }
        auto me = hint.me;

        while (true) {
            if (elem < me->value) {
                if (!me->leftEl) return static_cast<const Type>(*(me->leftEl = alloc(me, std::forward<Type>(elem))));
                me = me->leftEl;
            } else {
                if (!me->rightEl) return static_cast<const Type>(*(me->rightEl = alloc(me, std::forward<Type>(elem))));
                me = me->rightEl;
            }
        }
    }

    const Type& emplace(Type&& elem) {//O(N) on empty tree or worst case. O(log2 N) on balanced tree
        ++elemCount; //#TODO we may reject duplicates
        if (!root) {
            root = alloc(nullptr, std::forward<Type>(elem));
            return root->value;
        }
        auto me = root;

        while (true) {
            if (elem < me->value) {
                if (!me->leftEl) return static_cast<const Type>(*(me->leftEl = alloc(me, std::forward<Type>(elem))));
                me = me->leftEl;
            } else {
                if (!me->rightEl) return static_cast<const Type>(*(me->rightEl = alloc(me, std::forward<Type>(elem))));
                me = me->rightEl;
            }
        }
    }

    const Type& insert(Type elem) {//O(N) on empty tree or worst case. O(log2 N) on balanced tree
        return emplace(std::forward<Type>(elem));//#TODO we really want to move here.. But we can't move uint32_t
    }

    const Type& insert(const iterator& hint, Type elem) {//O(N) on empty tree or worst case. O(log2 N) on balanced tree
        return emplace(hint, std::forward<Type>(elem));//#TODO we really want to move here.. But we can't move uint32_t
    }


    void remove(Type elem) {//Same as insert O(N) to O(log2 N) to find element. If element has 2 subelements then another insert with O(N) to O(log2 N)
        if (!root) return;
        auto me = root;

        while (true) {
            if (me->value != elem) {
                if (elem < me->value && me->leftEl) {
                    me = me->leftEl;
                    continue;
                }
                if (elem > me->value && me->rightEl) {
                    me = me->rightEl;
                    continue;
                }
                return; //elem doesn't exist
            }



            if (!me->parent) {//We are root
                if (!me->leftEl && !me->rightEl) {//No sub elements. Just delete us.
                    deAlloc(me);
                    root = nullptr;
                    --elemCount;
                    return;
                }
                if (!me->leftEl) {//One sub element. Just move it up.
                    me->rightEl->parent = nullptr;
                    root = me->rightEl;
                    me->rightEl = nullptr;
                    deAlloc(me);
                    --elemCount;
                    return;

                }
                if (!me->rightEl) {//One sub element. Just move it up.
                    me->leftEl->parent = nullptr;
                    root = me->leftEl;
                    me->leftEl = nullptr;
                    deAlloc(me);
                    --elemCount;
                    return;
                }


                //Two sub elements
                me->rightEl->parent = nullptr;
                root = me->rightEl; //move right elem to parent
                me->rightEl = nullptr;//moved away
                root->insertElement(me->leftEl);
                me->leftEl = nullptr;//moved away
                deAlloc(me);
                --elemCount;
                return;
            } else if (me->parent->leftEl == me) {//We are left elem of parent
                if (!me->leftEl && !me->rightEl) {//No sub elements. Just delete us.
                    me->parent->leftEl = nullptr;
                    deAlloc(me);
                    --elemCount;
                    return;
                }
                if (!me->leftEl) {//One sub element. Just move it up.
                    me->rightEl->parent = me->parent;
                    me->parent->leftEl = me->rightEl;
                    me->rightEl = nullptr;
                    deAlloc(me);
                    --elemCount;
                    return;

                }
                if (!me->rightEl) {//One sub element. Just move it up.
                    me->leftEl->parent = me->parent;
                    me->parent->leftEl = me->leftEl;
                    me->leftEl = nullptr;
                    deAlloc(me);
                    --elemCount;
                    return;
                }


                //Two sub elements
                me->rightEl->parent = me->parent;
                me->parent->leftEl = me->rightEl; //move right elem to parent
                me->rightEl = nullptr;//moved away
                root->insertElement(me->leftEl);
                me->leftEl = nullptr;//moved away
                deAlloc(me);
                --elemCount;
                return;
            } else {//we are right elem of parent
                if (!me->leftEl && !me->rightEl) {//No sub elements. Just delete us
                    me->parent->rightEl = nullptr;
                    deAlloc(me);
                    --elemCount;
                    return;
                }
                if (!me->leftEl) {//One sub element. Just move it up.
                    me->rightEl->parent = me->parent;
                    me->parent->rightEl = me->rightEl;
                    me->rightEl = nullptr;
                    deAlloc(me);
                    --elemCount;
                    return;

                }
                if (!me->rightEl) {//One sub element. Just move it up.
                    me->leftEl->parent = me->parent;
                    me->parent->rightEl = me->leftEl;
                    me->leftEl = nullptr;
                    deAlloc(me);
                    --elemCount;
                    return;
                }

                //Two sub elements
                me->rightEl->parent = me->parent;
                me->parent->rightEl = me->rightEl; //move right elem to parent
                me->rightEl = nullptr;
                root->insertElement(me->leftEl);
                me->leftEl = nullptr;
                deAlloc(me);
                --elemCount;
                return;
            }
        }
    }


    uint64_t count() const noexcept {//O(1)
        return elemCount;
    }
    Type minValue() const noexcept {//O(1) min. If left branch doesn't exist. O(depth) at max. Only traverses left
        if (!root) return 0;
        auto me = root;
        while (me->leftEl)
            me = me->leftEl;
        return me->value;
    }
    Type maxValue() const noexcept {//O(1) min. If right branch doesn't exist. O(depth) at max. Only traverses right
        if (!root) return 0;
        auto me = root;
        while (me->rightEl)
            me = me->rightEl;
        return me->value;
    }
    bool contains(const Type& searchVal) {//(log2 N) to O(N) traverses just like insert
        if (!root) return false;
        auto me = root;
        while (me->value != searchVal && (me->leftEl || me->rightEl)) {
            if (searchVal < me->value && me->leftEl) return me = me->leftEl;
            if (searchVal > me->value && me->rightEl) return me = me->rightEl;
        }
        return (me->value == searchVal);
    }

    iterator begin() {
        return iterator(*this);
    }
    iterator end() {
        return iterator(*this, nullptr);
    }
    bool empty() {
        return root == nullptr;
    }
    void swap(bintree<Type>& other) {
        auto holder = root;
        root = other.root;
        other.root = holder;
    }

};













template <class char_t>
size_t tosize(char_t const& aChar) {
    union cvt {
        char_t c{1};
        size_t s;
        char test[50];
    }c ;
    c.c = aChar;
    return c.s;
}



int main() {
    auto s = tosize<char>('a');
    return s;

    bintree<uint32_t, poolAllocator<bintreeElement<uint32_t>>> tr;
    //uint32_t mid = 20;
    //for (uint32_t i = mid + 1; mid > 0; ++i) {
    //    tr.insert(mid);
    //    tr.insert(i);
    //    --mid;
    //}

   for (uint32_t i = 01; i < 5500; ++i) {
       tr.insert(i);
   }
    tr.insert(50);

    tr.insert(25);
    tr.insert(75);


    tr.insert(25 + 12);
    tr.insert(25 - 12);

    tr.insert(75 + 12);
    tr.insert(75 - 12);


    tr.inOrder([](uint32_t el) {
        std::cout << el << "\n";
    });
    std::cout << "inIterator\n";
    //for (const auto& el : tr)
    //    std::cout << el << "\n";

    auto it = tr.begin();
    std::cout << *it << " = 13\n";
    it++;
    std::cout << *it << " = 25\n";
    it++;
    std::cout << *it << " = 37\n";
    it++;
    std::cout << *it << " = 50\n";
    it++;
    std::cout << *it << " = 63\nback\n";
    it--;
    std::cout << *it << " = 50\n";
    it--;
    std::cout << *it << " = 37\n";
    it--;
    std::cout << *it << " = 25\nforward\n";
    it++;
    std::cout << *it << " = 50\n";
    it++;
    std::cout << *it << " = 63\nback\n";
    it--;
    std::cout << *it << " = 50\n";
    it--;
    std::cout << *it << " = 37\n";
    it--;
    std::cout << *it << " = 25\n";


    std::cout << "inOrdBak\n";

    tr.inOrderBackwards([](uint32_t el) {
        std::cout << el << "\n";
    });
    std::cout << "post\n";
    tr.postOrder([](uint32_t el) {
        std::cout << el << "\n";
    });
    std::cout << "\n";
    std::cout << "pre\n";
    tr.preOrder([](uint32_t el) {
        std::cout << el << "\n";
    });
    std::cout << "\n";

    //tr.balance();
    auto c = tr.count();
    auto depth = tr.depth();
    std::cout << depth << " depth\n";
    std::cout << c << " elements\n";
    //tr.inOrder([](uint32_t el) {
    //    std::cout << el << "\n";
    //});
    std::cout << "rem 75\n";
    tr.remove(75);
    c = tr.count();
    depth = tr.depth();
    std::cout << depth << " depth\n";
    std::cout << c << " elements\n";
    tr.inOrder([](uint32_t el) {
        std::cout << el << "\n";
    });


    std::cout << "rem 75 + 12\n";
    tr.remove(75 + 12);
    c = tr.count();
    depth = tr.depth();
    std::cout << depth << " depth\n";
    std::cout << c << " elements\n";
    tr.inOrder([](uint32_t el) {
        std::cout << el << "\n";
    });
    std::cout << "rem stuffz (depth decrement)\n";
    tr.remove(75 - 12);
    tr.remove(25 + 12);
    tr.remove(25 - 12);
    c = tr.count();
    depth = tr.depth();
    std::cout << depth << " depth\n";
    std::cout << c << " elements\n";
    tr.inOrder([](uint32_t el) {
        std::cout << el << "\n";
    });



    getchar();
    return 0;
}

