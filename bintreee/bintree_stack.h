#pragma once

template<class Type>
class bintree {

    class bintreeElement {
        friend class bintree;

        bintreeElement* leftEl{ nullptr };
        bintreeElement* rightEl{ nullptr };
        Type value;

        static bintreeElement* alloc(Type&& initV) {
            return new bintreeElement(std::forward<Type>(initV));
        }

        static void deAlloc(bintreeElement* elem) {
            if (elem) delete elem;
        }
    public:
        explicit bintreeElement(Type&& v) : value(std::forward<Type>(v)) {}
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
                    return;
                }
                if (el->value > me->value && !me->rightEl) {
                    me->rightEl = el;
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

    bintreeElement* root;
    uint32_t elemCount = 0;
    /*
    stackless iterator using parent pointer in elements
    pool-allocator with freelist
    binary-heap container


    */
public:


    class iterator : public std::iterator<std::bidirectional_iterator_tag, Type> {
        std::stack<const bintreeElement*> stack{};
        const bintreeElement* me = nullptr;
        const bintree* tree;
        enum class StepType {
            norm,
            getRight,
            getLeft
        } curStep = StepType::norm;
        uint32_t backTrackCount = 0;
    public:
        iterator(const iterator& other) : stack(other.stack), me(other.me), curStep(other.curStep), tree(other.tree), backTrackCount(other.backTrackCount) {}
        explicit iterator(const bintree& bt) { tree = &bt; me = bt.root;  stack.push(bt.root); getNext(); }
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

            if (backTrackCount != 0) {
                --backTrackCount;
                me = stack.top();
                stack.pop();
                return;
            }

            if (curStep == StepType::getRight)
                me = me->rightEl;
            curStep = StepType::norm;
            while (!stack.empty()) {
                if (me != nullptr) {
                    stack.push(me);
                    me = me->leftEl;
                } else {
                    me = stack.top();
                    if (me == nullptr) return;//at end
                    stack.pop();

                    curStep = StepType::getRight;
                    return;
                }
            }
        }


        void getPrevious() {
            backTrackCount++;
            stack.push(me);
            me = tree->getPrevious(me);
        }

    };



private:

    const bintreeElement* getPrevious(const bintreeElement* current) const { //O(N)
        if (!root) return nullptr;
        std::stack<const bintreeElement*> stack;
        const bintreeElement* me = root;

        bool nextHit = (current == nullptr);//If element is invalid just return the last element instead. element would be invalid on end() iterator
        stack.push(nullptr);
        while (!stack.empty()) {
            if (me != nullptr) {
                stack.push(me);
                me = me->rightEl;
            } else {
                me = stack.top();
                if (me == nullptr) return nullptr;
                stack.pop();
                if (nextHit) return me;
                if (me == current) nextHit = true;


                me = me->leftEl;
            }
        }
        return nullptr;
    }
public:







    template <typename Func>
    void inOrder(Func func) { //O(N)
        if (!root) return;
        std::stack<bintreeElement*> stack;
        auto me = root;

        // while (me->leftEl) {//Go to leftmost element
        //     stack.push(me);
        //     me = me->leftEl;
        // }
        stack.push(nullptr);
        while (!stack.empty()) {
            if (me != nullptr) {
                stack.push(me);
                me = me->leftEl;
            } else {
                me = stack.top();
                if (me == nullptr) return;
                stack.pop();

                func(me->value);

                me = me->rightEl;
            }
        }
    }
    template <typename Func>
    void preOrder(Func func) { //O(N)
        if (!root) return;
        std::stack<bintreeElement*> stack;
        auto me = root;
        stack.push(nullptr);
        while (!stack.empty()) {
            if (me != nullptr) {
                func(me->value);
                stack.push(me);
                me = me->leftEl;
            } else {
                me = stack.top();
                if (me == nullptr) return;
                stack.pop();
                me = me->rightEl;
            }
        }
    }
    template <typename Func>
    void postOrder(Func func) { //O(N)
        if (!root) return;
        std::stack<bintreeElement*> stack;
        auto me = root;
        auto last = root;
        while (me->leftEl) {//Go to leftmost element
            stack.push(me);
            me = me->leftEl;
        }

        while (!stack.empty()) {
            if (me != nullptr) {
                stack.push(me);
                me = me->leftEl;
            } else {
                me = stack.top();
                stack.pop();

                if (last == me->rightEl) {
                    last = me;
                    func(me->value);
                    me = nullptr;
                } else {
                    stack.push(me);
                    me = me->rightEl;
                    last = nullptr;
                }
            }
        }
    }

    template <typename Func>
    void inOrderBackwards(Func func) { //O(N)
        if (!root) return;
        std::stack<bintreeElement*> stack;
        auto me = root;

        // while (me->leftEl) {//Go to leftmost element
        //     stack.push(me);
        //     me = me->leftEl;
        // }
        stack.push(nullptr);
        while (!stack.empty()) {
            if (me != nullptr) {
                stack.push(me);
                me = me->rightEl;
            } else {
                me = stack.top();
                if (me == nullptr) return;
                stack.pop();

                func(me->value);

                me = me->leftEl;
            }
        }
    }

    size_t depth() {//O(N) visits every element
        if (!root) return 0;
        size_t maxLevel = 0;
        //just a copy of inOrder...
        std::stack<const bintreeElement*> stack;
        const bintreeElement* me = root;

        // while (me->leftEl) {//Go to leftmost element
        //     stack.push(me);
        //     me = me->leftEl;
        // }
        stack.push(nullptr);
        while (!stack.empty()) {
            if (me != nullptr) {
                stack.push(me);
                me = me->leftEl;
            } else {
                me = stack.top();
                if (me == nullptr) return maxLevel;
                stack.pop();

                maxLevel = std::max(stack.size(), maxLevel);

                me = me->rightEl;
            }
        }
        return maxLevel;
    }




    const Type& emplace(const iterator& hint, Type&& elem) {//O(N) on empty tree or worst case. O(log2 N) on balanced tree
        ++elemCount; //#TODO we may reject duplicates
        if (!root) {
            root = bintreeElement::alloc(std::forward<Type>(elem));
            return root->value;
        }
        auto me = hint.me;

        while (true) {
            if (elem < me->value) {
                if (!me->leftEl) return static_cast<const Type>(*(me->leftEl = bintreeElement::alloc(std::forward<Type>(elem))));
                me = me->leftEl;
            } else {
                if (!me->rightEl) return static_cast<const Type>(*(me->rightEl = bintreeElement::alloc(std::forward<Type>(elem))));
                me = me->rightEl;
            }
        }
    }


    const Type& emplace(Type&& elem) {//O(N) on empty tree or worst case. O(log2 N) on balanced tree
        ++elemCount; //#TODO we may reject duplicates
        if (!root) {
            root = bintreeElement::alloc(std::forward<Type>(elem));
            return root->value;
        }
        auto me = root;

        while (true) {
            if (elem < me->value) {
                if (!me->leftEl) return static_cast<const Type>(*(me->leftEl = bintreeElement::alloc(std::forward<Type>(elem))));
                me = me->leftEl;
            } else {
                if (!me->rightEl) return static_cast<const Type>(*(me->rightEl = bintreeElement::alloc(std::forward<Type>(elem))));
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

    //void balance(/*std::optional<Type> to_remove = {}*/) {//broken
    //    std::vector<Type> elements;
    //    inOrder([&elements](Type el) {
    //        elements.emplace_back(el);
    //    });
    //    bool wasSorted = std::is_sorted(elements.begin(), elements.end());
    //    if (!wasSorted) __debugbreak();
    //    //if (to_remove) std::remove_if(elements.begin(), elements.end(), [&to_remove](Type el) {
    //    //    return (el == *to_remove);
    //    //});
    //    //reinsert
    //    root = std::make_shared<bintreeElement>(0);
    //    insSortedVec(root, elements, 0, elements.size() - 1);
    //
    //}



    void remove(Type elem) {//Same as insert O(N) to O(log2 N) to find element. If element has 2 subelements then another insert with O(N) to O(log2 N)
        if (!root) return;

        auto parent = root;
        auto me = root;

        while (true) {
            if (me->value != elem) {
                if (elem < me->value && me->leftEl) {
                    parent = me;
                    me = me->leftEl;
                    continue;
                }
                if (elem > me->value && me->rightEl) {
                    parent = me;
                    me = me->rightEl;
                    continue;
                }
                return; //elem doesn't exist
            }
            if (parent->leftEl == me) {//We are left elem of parent
                if (!me->leftEl && !me->rightEl) {//No sub elements. Just delete us.
                    parent->leftEl = nullptr;
                    delete me;
                    --elemCount;
                    return;
                }
                if (!me->leftEl) {//One sub element. Just move it up.
                    parent->leftEl = me->rightEl;
                    me->rightEl = nullptr;
                    delete me;
                    --elemCount;
                    return;

                }
                if (!me->rightEl) {//One sub element. Just move it up.
                    parent->leftEl = me->leftEl;
                    me->leftEl = nullptr;
                    delete me;
                    --elemCount;
                    return;
                }


                //Two sub elements
                parent->leftEl = me->rightEl; //move right elem to parent
                me->rightEl = nullptr;//moved away
                root->insertElement(me->leftEl);
                me->leftEl = nullptr;//moved away
                delete me;
                --elemCount;
                return;
            } else {//we are right elem of parent
                if (!me->leftEl && !me->rightEl) {//No sub elements. Just delete us
                    parent->rightEl = nullptr;
                    delete me;
                    --elemCount;
                    return;
                }
                if (!me->leftEl) {//One sub element. Just move it up.
                    parent->rightEl = me->rightEl;
                    me->rightEl = nullptr;
                    delete me;
                    --elemCount;
                    return;

                }
                if (!me->rightEl) {//One sub element. Just move it up.
                    parent->rightEl = me->leftEl;
                    me->leftEl = nullptr;
                    delete me;
                    --elemCount;
                    return;
                }

                //Two sub elements
                parent->rightEl = me->rightEl; //move right elem to parent
                me->rightEl = nullptr;
                root->insertElement(me->leftEl);
                me->leftEl = nullptr;
                delete me;
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



