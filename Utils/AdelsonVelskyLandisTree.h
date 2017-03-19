#pragma once
#include <assert.h>
#include <memory>
#include <deque>
// T - data type
// K - key type
// A - allocator
// P - predicate

template<typename K> struct avl_tree_predicate
{
	int operator()(K const &k1, K const &k2) const
	{
		if (k1 < k2)
		{
			return -1;
		}
		else if (k1 > k2)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
};

template<typename T, typename K> class avl_tree_base
{
	// this class defines iterators instantiated on data type and key basis,
	// independently of tree predicate and allocator.
public:
protected:
	struct node;
	struct node_header
	{
		union
		{
			struct
			{
				node * left;
				node * right;
				node * parent;
			};
			struct
			{
				// rightmost overlays with 'left' pointer, and 'leftmost' overlays with 'right' pointer
				node * rightmost;
				node * leftmost;
				node * root;
			};
		};

		bool is_left;
		bool is_terminal;   // special node to use as enther end() or rend()

		signed char node_balance;   // 0 - left and right balanced. -1 - left tree higher. 1 - right tree higher

		node_header(node * _parent)
			:left(nullptr)
			, right(nullptr)
			, parent(_parent)
			, node_balance(0)
			, is_left(false)
			, is_terminal(false)
		{
		}
	};

	struct node : node_header
	{
		T data;
		K key;

		node * prev() const;  // enumerate from right to left
		node * next() const;  // enumerate from left to right

		node(node * _parent, T const&_t, K const &_key)
			: node_header(_parent)
			, data(_t)
			, key(_key)
		{
		}
	};

public:
	class const_iterator;
	class iterator
	{
	public:
		K const& key() const
		{
			assert(!p->is_terminal);
			return p->key;
		}
		T * operator->() const
		{
			assert(!p->is_terminal);
			return &p->data;
		}
		T & operator*() const
		{
			assert(!p->is_terminal);
			return p->data;
		}
		operator void const *() const
		{
			return p;
		}
		bool operator ==(iterator const&a) const
		{
			return p == a.p;
		}
		bool operator !=(iterator const&a) const
		{
			return p != a.p;
		}

		bool operator ==(const_iterator const&a) const;
		bool operator !=(const_iterator const&a) const;

		iterator &operator++()  // pre-increment
		{
			assert(!p->is_terminal);
			p = p->next();
			return *this;
		}
		iterator operator++(int)  // post-increment
		{
			node * _p = p;
			assert(!p->is_terminal);
			p = p->next();
			return _p;
		}
		iterator &operator--()  // pre-decrement
		{
			if (p->is_terminal)
			{
				// decrement of end iterator yields hdr.rightmost
				assert(p->rightmost != p);
				p = p->rightmost;
			}
			else
			{
				p = p->prev();
			}
			return *this;
		}
		iterator operator--(int)  // post-decrement
		{
			node * _p = p;
			if (p->is_terminal)
			{
				// decrement of end iterator yields hdr.rightmost
				assert(p->rightmost != p);
				p = p->rightmost;
			}
			else
			{
				p = p->prev();
			}
			return _p;
		}
		iterator()
			:p(nullptr)
		{}

	protected:
		iterator(node * _p)
			: p(_p)
		{

		}
		node * p;
		template<typename T1, typename K1, typename P, typename A> friend class avl_tree;
		friend class const_iterator;
	};

	class const_iterator : protected iterator
	{
		friend class iterator;
	public:
		K const& key() const
		{
			assert(!p->is_terminal);
			return p->key;
		}
		T const * operator->() const
		{
			assert(!p->is_terminal);
			return &p->data;
		}
		T const & operator*() const
		{
			assert(!p->is_terminal);
			return p->data;
		}
		operator void const *() const
		{
			return p;
		}
		bool operator ==(const_iterator const&a) const
		{
			return p == a.p;
		}
		bool operator !=(const_iterator const&a) const
		{
			return p != a.p;
		}

		bool operator ==(iterator const&a) const
		{
			return p == a.p;
		}
		bool operator !=(iterator const&a) const
		{
			return p != a.p;
		}

		const_iterator &operator++()  // pre-increment
		{
			iterator::operator++();
			return *this;
		}
		const_iterator operator++(int)  // post-increment
		{
			return iterator::operator++(0);
		}
		const_iterator &operator--()  // pre-decrement
		{
			iterator::operator--();
			return *this;
		}
		const_iterator operator--(int)  // post-decrement
		{
			return iterator::operator--(0);
		}

		const_iterator(iterator const &i)
			: iterator(i)
		{

		}
		const_iterator()
		{}

	protected:
		const_iterator(node const * _p)
			: iterator(const_cast<node *>(_p))
		{

		}
		template<typename T1, typename K1, typename P, typename A> friend class avl_tree;
	};

	class const_reverse_iterator;
	class reverse_iterator
	{
	public:
		K const& key() const
		{
			assert(!p->is_terminal);
			return p->key;
		}
		T * operator->() const
		{
			assert(!p->is_terminal);
			return &p->data;
		}
		T & operator*() const
		{
			assert(!p->is_terminal);
			return p->data;
		}
		operator void const *() const
		{
			return p;
		}
		bool operator ==(reverse_iterator const&a) const
		{
			return p == a.p;
		}
		bool operator !=(reverse_iterator const&a) const
		{
			return p != a.p;
		}

		bool operator ==(const_reverse_iterator const&a) const;
		bool operator !=(const_reverse_iterator const&a) const;

		reverse_iterator &operator++()  // pre-increment
		{
			assert(!p->is_terminal);
			p = p->prev();
			return *this;
		}
		reverse_iterator operator++(int)  // post-increment
		{
			node * _p = p;
			assert(!p->is_terminal);
			p = p->prev();
			return _p;
		}
		reverse_iterator &operator--()  // pre-decrement
		{
			if (p->is_terminal)
			{
				// decrement of rend iterator yields hdr.lefttmost
				assert(p->leftmost != p);
				p = p->leftmost;
			}
			else
			{
				p = p->next();
			}
			return *this;
		}
		reverse_iterator operator--(int)  // post-decrement
		{
			node * _p = p;
			if (p->is_terminal)
			{
				// decrement of end iterator yields hdr.rightmost
				assert(p->leftmost != p);
				p = p->leftmost;
			}
			else
			{
				p = p->next();
			}
			return _p;
		}
		reverse_iterator()
			:p(nullptr)
		{}

	protected:
		reverse_iterator(node * _p)
			: p(_p)
		{

		}
		node * p;
		template<typename T1, typename K1, typename P, typename A> friend class avl_tree;
		friend class const_reverse_iterator;
	};

	class const_reverse_iterator : protected reverse_iterator
	{
		friend class reverse_iterator;
	public:
		K const& key() const
		{
			assert(!p->is_terminal);
			return p->key;
		}
		T const * operator->() const
		{
			assert(!p->is_terminal);
			return &p->data;
		}
		T const & operator*() const
		{
			assert(!p->is_terminal);
			return p->data;
		}
		operator void const *() const
		{
			return p;
		}
		bool operator ==(const_reverse_iterator const&a) const
		{
			return p == a.p;
		}
		bool operator !=(const_reverse_iterator const&a) const
		{
			return p != a.p;
		}

		bool operator ==(reverse_iterator const&a) const
		{
			return p == a.p;
		}
		bool operator !=(reverse_iterator const&a) const
		{
			return p != a.p;
		}

		const_reverse_iterator &operator++()  // pre-increment
		{
			reverse_iterator::operator++();
			return *this;
		}
		const_reverse_iterator operator++(int)  // post-increment
		{
			return reverse_iterator::operator++(0);
		}
		const_reverse_iterator &operator--()  // pre-decrement
		{
			reverse_iterator::operator--();
			return *this;
		}
		const_reverse_iterator operator--(int)  // post-decrement
		{
			return reverse_iterator::operator--(0);
		}

		const_reverse_iterator(reverse_iterator const &i)
			: reverse_iterator(i)
		{

		}
		const_reverse_iterator()
		{}

	protected:
		const_reverse_iterator(node const * _p)
			: reverse_iterator(const_cast<node *>(_p))
		{

		}
		template<typename T1, typename K1, typename P, typename A> friend class avl_tree;
	};

};

template<typename T, typename K, typename P= avl_tree_predicate<K>, typename A=std::allocator<T> > class avl_tree : protected avl_tree_base<T, K>
{
public:
	using typename avl_tree_base::iterator;
	using typename avl_tree_base::const_iterator;
	using typename avl_tree_base::reverse_iterator;
	using typename avl_tree_base::const_reverse_iterator;

	iterator insert(K const &key, T const& t = T())
	{
		return insert_node(key, t);
	}

	iterator insert(K const &key, T const& t, iterator hint)
	{
		if (hint != iterator(nullptr)
			&& 0 == _predicate(key, hint->key))
		{
			return hint;
		}

		return insert_node(key, t);
	}

	iterator find(K const &key)    // returns end() if not found
	{
		return find_node(key);
	}
	const_iterator find(K const &key) const    // returns end() if not found
	{
		return find_node(key);
	}
	bool remove(K const &key)
	{
		node * nn = find_node(key);
		if (nn == &hdr)
		{
			return false;
		}
		erase_node(nn);
		return true;
	}

	void erase(iterator ii)
	{
		erase_node(ii.p);
	}
	void reset();   // delete all

	void validate() const;  // check the tree for validity

	iterator begin()
	{
		return hdr.leftmost;
	}
	iterator end()
	{
		return static_cast<node*>(&hdr);
	}

	const_iterator begin() const
	{
		return hdr.leftmost;
	}
	const_iterator end() const
	{
		return static_cast<node const*>(&hdr);
	}

	const_iterator cbegin() const
	{
		return hdr.leftmost;
	}
	const_iterator cend() const
	{
		return static_cast<node const*>(&hdr);
	}

	reverse_iterator rbegin()
	{
		return hdr.rightmost;
	}
	reverse_iterator rend()
	{
		return static_cast<node*>(&hdr);
	}

	const_reverse_iterator rbegin() const
	{
		return hdr.rightmost;
	}
	const_reverse_iterator rend() const
	{
		return static_cast<node const*>(&hdr);
	}

	const_reverse_iterator crbegin() const
	{
		return hdr.rightmost;
	}

	const_reverse_iterator crend() const
	{
		return static_cast<node const*>(&hdr);
	}

	size_t size() const
	{
		return tree_count;
	}

	P const & predicate() const
	{
		return _predicate;
	}

	avl_tree(P const & _pr = P(), A const & a = A())
		: _predicate(_pr), allocator(a), hdr(nullptr)
		, tree_count(0)
		, tree_height(0)
	{
		hdr.is_terminal = true;
		hdr.leftmost = static_cast<node*>(&hdr);
		hdr.rightmost = static_cast<node*>(&hdr);
	}

	template<typename T1, typename K1, typename P1 = avl_tree_predicate<K1>, typename A1 = std::allocator<T1> >
	avl_tree &operator=(avl_tree<T1, K1, P1, A1> const & src);

	avl_tree &operator=(avl_tree const & src);

	avl_tree(avl_tree const & src)
		: avl_tree(src._predicate, src.allocator)
	{
		operator=(src);
	}

	~avl_tree()
	{
		reset();
	}

private:
	using typename avl_tree_base::node;
	node *find_node(K const &key) const;    // returns nullptr if not found
	node *insert_node(K const &key, T const& t);
	bool remove_node(node *leaf, node **pp_top_node);
	void erase_node(node *leaf);

	void validate_subtree(node const * tree, int * depth, int * number_nodes, node const** subtree_leftmost, node const** subtree_rightmost) const;

	node_header hdr;
	size_t tree_count;
	unsigned tree_height;

	A allocator;
	P _predicate;
};

template<typename T, typename K> inline bool avl_tree_base<T, K>::iterator::operator==(const_iterator const&a) const
{
	return p == a.p;
}

template<typename T, typename K> inline bool avl_tree_base<T, K>::iterator::operator!=(const_iterator const&a) const
{
	return p != a.p;
}

template<typename T, typename K> inline bool avl_tree_base<T, K>::reverse_iterator::operator==(const_reverse_iterator const&a) const
{
	return p == a.p;
}

template<typename T, typename K> inline bool avl_tree_base<T, K>::reverse_iterator::operator!=(const_reverse_iterator const&a) const
{
	return p != a.p;
}

template<typename T, typename K> typename avl_tree_base<T, K>::node * avl_tree_base<T, K>::node::prev() const
{
	assert(!is_terminal);
	node const * leaf = this;
	node *left_leaf = leaf->left;

	// if current node has left branch, return the rightmost leaf of it
	if (left_leaf != nullptr)
	{
		// go to the rightmost leaf of the left node
		while (left_leaf->right != nullptr)
		{
			left_leaf = left_leaf->right;
		}
		return left_leaf;
	}
	// If current node is right, return its parent.
	node * parent_node;
	while (1)
	{
		parent_node = leaf->parent;
		if (!leaf->is_left || parent_node->is_terminal)
		{
			return parent_node;
		}
		leaf = parent_node;
	}
}

template<typename T, typename K> typename avl_tree_base<T, K>::node * avl_tree_base<T, K>::node::next() const
{
	assert(!is_terminal);
	node const * leaf = this;
	node *right_leaf = leaf->right;

	// if current node has right branch, return the leftmost leaf of it
	if (right_leaf != nullptr)
	{
		// go to the leftmost leaf of the right node
		while (right_leaf->left != nullptr)
		{
			right_leaf = right_leaf->left;
		}
		return right_leaf;
	}
	// If current node is left, return its parent.
	node * parent_node;
	while (1)
	{
		parent_node = leaf->parent;
		if (leaf->is_left || parent_node->is_terminal)
		{
			return parent_node;
		}
		leaf = parent_node;
	}
}

template<typename T, typename K, typename P, typename A> typename avl_tree_base<T, K>::node * avl_tree<T, K, P, A>::find_node(K const &key) const
{
	node * leaf = hdr.root;
	while (leaf != nullptr)
	{
		int compare_result = _predicate(key, leaf->key);

		if (compare_result < 0)
		{
			leaf = leaf->left;
		}
		else if (compare_result > 0)
		{
			leaf = leaf->right;
		}
		else
		{
			return leaf;
		}
	}
	return const_cast<node*>(static_cast<node const*>(&hdr));
}

template<typename T, typename K, typename P, typename A> typename avl_tree_base<T, K>::node * avl_tree<T, K, P, A>::insert_node(K const &key, T const& t)
{
	node * leaf = hdr.root;
	node * new_node;

	if (leaf == nullptr)
	{
		new_node = new node(static_cast<node*>(&hdr), t, key);
		tree_count++;
		tree_height++;
		hdr.leftmost = new_node;
		hdr.rightmost = new_node;
		hdr.root = new_node;
		return new_node;
	}

	while (1)
	{
		int compare_result = _predicate(key, leaf->key);
		if (compare_result < 0)
		{
			if (leaf->left != nullptr)
			{
				leaf = leaf->left;
				continue;
			}
			// insert new node to leaf->left
			new_node = new node(leaf, t, key);
			leaf->left = new_node;
			new_node->is_left = true;
			tree_count++;
			if (leaf == hdr.leftmost)
			{
				hdr.leftmost = new_node;
			}
			leaf->node_balance--;
			if (leaf->node_balance >= 0)
			{
				// the tree is still balanced
				return new_node;
			}
			// the tree branch grew by one. This could cause tree balance violation
			break;
		}
		else if (compare_result > 0)
		{
			if (leaf->right != nullptr)
			{
				leaf = leaf->right;
				continue;
			}
			new_node = new node(leaf, t, key);
			leaf->right = new_node;
			new_node->is_left = false;

			tree_count++;
			if (leaf == hdr.rightmost)
			{
				hdr.rightmost = new_node;
			}

			leaf->node_balance++;
			if (leaf->node_balance <= 0)
			{
				// the tree is still balanced
				return new_node;
			}
			// the tree branch grew by one. This could cause tree balance violation
			break;
		}
		else
		{
			return leaf;
		}
	}

	// we get here if the leaf branch grew by one leaf in height
	// go up and see if it caused balance violation
	// leaf is a pointer to the node where the new node was just added. Its height grew by one, because it used to
	// be a terminal node before, and now it's got abother leaf.
	// on each next iteration, 'leaf' is set to its parent
	// Node A is the current node that needs to be checked for balance
	// 'parent' is its parent node (or nullptr)
	for (node * A = leaf->parent, *parent; !A->is_terminal; leaf = A, A = A->parent)
	{
		node * new_top;

		if (leaf->is_left)
		{
			A->node_balance--;
			if (A->node_balance == 0)
			{
				// the tree is still balanced
				return new_node;
			}
			if (A->node_balance == -1)
			{
				// continue checking balance
				continue;
			}

			// the node became disbalanced (node_balance = -2). Left branch is too heavy. Apply rotation operation
			node * B = A->left;
			if (B->node_balance < 0)
			{
				node * D = B->right;
				A->left = D;
				if (D != nullptr)
				{
					D->parent = A;
					D->is_left = true;
				}

				A->node_balance = 0;
				B->node_balance = 0;

				B->right = A;
				B->is_left = A->is_left;
				B->parent = A->parent;

				A->parent = B;
				A->is_left = false;

				new_top = B;
			}
			else
			{
				node * C = B->right;

				node * F = C->left;
				B->right = F;
				if (F != nullptr)
				{
					F->parent = B;
					F->is_left = false;
				}

				node * G = C->right;
				A->left = G;
				if (G != nullptr)
				{
					G->parent = A;
					G->is_left = true;
				}

				C->is_left = A->is_left;
				C->parent = A->parent;

				C->left = B;
				C->right = A;

				B->parent = C;
				A->parent = C;

				B->is_left = true;
				A->is_left = false;

				if (C->node_balance > 0)
				{
					A->node_balance = 0;
					B->node_balance = -1;
				}
				else if (C->node_balance < 0)
				{
					A->node_balance = 1;
					B->node_balance = 0;
				}
				else
				{
					// this happens if both become terminal leafs (C was a newly inserted node)
					assert(A->left == nullptr);
					assert(A->right == nullptr);
					assert(B->left == nullptr);
					assert(B->right == nullptr);
					A->node_balance = 0;
					B->node_balance = 0;
				}

				C->node_balance = 0;

				new_top = C;
			}
		}
		else
		{
			A->node_balance++;
			if (A->node_balance == 0)
			{
				// the tree is still balanced
				return new_node;
			}
			if (A->node_balance == 1)
			{
				// continue checking balance
				continue;
			}

			// the node became disbalanced. Right branch is too heavy. Apply rotation operation
			node * B = A->right;
			if (B->node_balance > 0)
			{
				node * D = B->left;
				A->right = D;
				if (D != nullptr)
				{
					D->parent = A;
					D->is_left = false;
				}

				A->node_balance = 0;
				B->node_balance = 0;

				B->left = A;
				B->is_left = A->is_left;
				B->parent = A->parent;

				A->parent = B;
				A->is_left = true;

				new_top = B;
			}
			else
			{
				node * C = B->left;

				node * F = C->right;
				B->left = F;
				if (F != nullptr)
				{
					F->parent = B;
					F->is_left = true;
				}

				node * G = C->left;
				A->right = G;
				if (G != nullptr)
				{
					G->parent = A;
					G->is_left = false;
				}

				C->is_left = A->is_left;
				C->parent = A->parent;

				C->right = B;
				C->left = A;

				B->parent = C;
				A->parent = C;

				B->is_left = false;
				A->is_left = true;

				if (C->node_balance < 0)
				{
					A->node_balance = 0;
					B->node_balance = 1;
				}
				else if (C->node_balance > 0)
				{
					A->node_balance = -1;
					B->node_balance = 0;
				}
				else
				{
					// this happens if both become terminal leafs (C was a newly inserted node)
					assert(A->left == nullptr);
					assert(A->right == nullptr);
					assert(B->left == nullptr);
					assert(B->right == nullptr);
					A->node_balance = 0;
					B->node_balance = 0;
				}

				C->node_balance = 0;
				new_top = C;
			}
		}

		// if we got here, the tree is rebalanced, and its height not changed
		parent = new_top->parent;
		if (parent->is_terminal)
		{
			hdr.root = new_top;
		}
		else if (new_top->is_left)
		{
			parent->left = new_top;
		}
		else
		{
			parent->right = new_top;
		}

		return new_node;
	}
	// if we got here, the tree is still balanced, but its height grew up by one
	tree_height++;
	return new_node;
}

template<typename T, typename K, typename P, typename A> void avl_tree<T, K, P, A>::erase_node(node *leaf)
{
	assert(!leaf->is_terminal);
	if (leaf == hdr.leftmost)
	{
		hdr.leftmost = leaf->next();
	}
	if (leaf == hdr.rightmost)
	{
		hdr.rightmost = leaf->prev();
	}
	if (remove_node(leaf, &hdr.root))
	{
		tree_height--;
	}
	delete leaf;
	tree_count--;
}

template<typename T, typename K, typename P, typename A> void avl_tree<T, K, P, A>::reset()
{
	// delete all
	while (tree_count != 0)
	{
		erase_node(hdr.leftmost);
	}
}

// returns 'true' if tree height reduced (by 1 only, never 2 or more)
// the function removes a node from a subtree, but doesn't delete it. The node may be needed later to reinsert to another position
// any rebalancing only done for the subtree
template<typename T, typename K, typename P, typename A> bool avl_tree<T, K, P, A>::remove_node(node *leaf, node **pp_top_node)
{
	// relink child nodes
	// use taller side as new top of tree
	node * new_top = nullptr;
	node * parent_node = leaf->parent;
	bool subtree_height_reduced = true;

	if (leaf->left != nullptr
		|| leaf->right != nullptr)
	{
		if (leaf->node_balance < 0)
		{
			// use hdr.rightmost node of the left subtree as the new top. Detach it first (having the subtree rebalanced)
			new_top = leaf->left;
			// find the hdr.rightmost node
			while (new_top->right != nullptr)
			{
				new_top = new_top->right;
			}
			// remove it from the subtree
			subtree_height_reduced = remove_node(new_top, &leaf->left);
			new_top->node_balance = leaf->node_balance + subtree_height_reduced;
			// the leaf to remove was left heavy.
			// If its height was reduced when the new_top subnode was removed, then the node becomes neutral balanced,
			// and its subtree height gets reduced, as well.
			subtree_height_reduced = (new_top->node_balance == 0);
		}
		else
		{
			// use hdr.leftmost node of the left subtree as the new top
			// use hdr.rightmost node of the right subtree as the new top. Detach it first (having the subtree rebalanced)
			new_top = leaf->right;
			// find the hdr.rightmost node
			while (new_top->left != nullptr)
			{
				new_top = new_top->left;
			}
			// remove it from the subtree
			subtree_height_reduced = remove_node(new_top, &leaf->right);
			new_top->node_balance = leaf->node_balance - subtree_height_reduced;
			// the leaf to remove was right heavy or balanced.
			// If its height was reduced when the new_top subnode was removed, then the node becomes neutral balanced,
			// and its subtree height gets reduced, as well.
			subtree_height_reduced = subtree_height_reduced && (new_top->node_balance == 0);
		}

		// now replace the removed node with new_top node
		new_top->is_left = leaf->is_left;
		new_top->parent = leaf->parent;
		new_top->right = leaf->right;
		if (new_top->right != nullptr)
		{
			new_top->right->parent = new_top;
		}
		new_top->left = leaf->left;
		if (new_top->left != nullptr)
		{
			new_top->left->parent = new_top;
		}
	}
	else
	{
		// leaf->node_balance == 0 && leaf->right == nullptr
		// this is the terminal leaf (no more leaves). Only need to detach from parent, and then rebalance the upper subtree, if necessary
		// subtree_height_reduced = true;
		// new_top = nullptr;
	}

	if (*pp_top_node == leaf)
	{
		*pp_top_node = new_top;
		return subtree_height_reduced;
	}
	if (leaf->is_left)
	{
		parent_node->left = new_top;
	}
	else
	{
		parent_node->right = new_top;
	}
	if (! subtree_height_reduced)
	{
		// this subtree is still the same height.
		return false;
	}

	// if we get here, this subtree reduced in height. Walk up the tree from 'leaf' till *pp_top_node and check if the tree is still balanced
	for (node * curr_node = leaf; curr_node != *pp_top_node; parent_node = curr_node->parent)
	{
		node * A = parent_node;
		node * C;
		node * D;
		if (curr_node->is_left)
		{
			if (++parent_node->node_balance == 1)
			{
				// parent node become right heavy, but parent tree height unchanged
				return false;
			}

			if (parent_node->node_balance == 0)
			{
				// parent node is still balanced, and parent tree height reduced
				curr_node = parent_node;
				continue;
			}
			assert(parent_node->node_balance == 2);

			// the parent node become unbalanced (right-over-heavy)
			/*
			As with rebalance during insertion, this rebalance has two possible cases:
												A
											   / \ 
											  /   \ 
											 /     \ 
											B       C
											|      / \ 
											|     /   \ 
											|    D     E
												/ \    |
											   F   G   |
											   |   |
			If the tree A (parent_node) is right over-heavy, subtree B (parent_node->left) has height 2 less than subtree C (parent_node->right).
			Two cases need to be considered:
			1. Subtree C is balanced or right-heavy
			2. Subtree C is left-heavy.

			In the first case, node C becomes top node, node A becomes C->left, and node D becomes A->right.
			If node C was balanced, A becomes right heavy, C becomes left heavy, and the subtree doesn't change height. Can return 0.
			If node C was right-heavy, A becomes balanced, C becomes balanced, and the subtree height is reduced by 1. Continue tree check.

			In the second case (C is left-heavy), D becomes top node, A becomes D->left, F becomes A->right, C becomes D->right, G becomes C->left.
			The tree height reduces by one, D is balanced, A is either balanced or left-heavy, C is either balanced or right heavy
			*/
			C = A->right;
			D = C->left;
			if (C->node_balance >= 0)
			{
				C->left = A;
				C->is_left = A->is_left;
				A->is_left = true;
				C->parent = A->parent;
				A->right = D;
				A->parent = C;
				if (*pp_top_node == A)
				{
					*pp_top_node = C;
				}
				else if (C->is_left)
				{
					C->parent->left = C;
				}
				else
				{
					C->parent->right = C;
				}

				if (D != nullptr)
				{
					D->is_left = false;
					D->parent = A;
				}
				if (C->node_balance == 0)
				{
					A->node_balance = 1;
					C->node_balance = -1;
					return 0;
				}

				A->node_balance = 0;
				C->node_balance = 0;
				// continue tree height check
				curr_node = C;
				continue;
			}
			else
			{
				/*
				In the second case (C is left - heavy), D becomes top node, A becomes D->left, F becomes A->right, C becomes D->right, G becomes C->left.
				The tree height reduces by one, D is balanced, A is either balanced or left-heavy, C is either balanced or right-heavy.
				*/
				node * F = D->left;
				node * G = D->right;
				D->right = C;
				D->left = A;

				A->right = F;
				C->left = G;

				A->node_balance = -(D->node_balance > 0);
				C->node_balance = (D->node_balance < 0);
				D->node_balance = 0;
				if (F != nullptr)
				{
					F->parent = A;
					F->is_left = false;
				}
				if (G != nullptr)
				{
					G->parent = C;
					G->is_left = true;
				}
				D->parent = A->parent;
				D->is_left = A->is_left;
				A->is_left = true;
				C->is_left = false;
				// fall through to the tail adjustment of the top node
				//A->parent = D;
				//C->parent = D;
			}
		}
		else
		{
			if (--parent_node->node_balance == -1)
			{
				// parent node become left heavy, but parent tree height unchanged
				return false;
			}

			if (parent_node->node_balance == 0)
			{
				// parent node is still balanced, and parent tree height reduced
				curr_node = parent_node;
				continue;
			}
			assert(parent_node->node_balance == -2);
			// the parent node become unbalanced (left-over-heavy)
			/*
			As with rebalance during insertion, this rebalance has two possible cases:
												 A
												/ \ 
											   /   \ 
											  /     \ 
											 C       B
											/ \      |
										   /   \     |
										  E     D
										  |    / \ 
										  |   G   F
											  |   |
			If the tree A (parent_node) is left over-heavy, subtree B (parent_node->right) has height 2 less than subtree C (parent_node->left).
			Two cases need to be considered:
			1. Subtree C is balanced or left-heavy
			2. Subtree C is right-heavy.

			In the first case, node C becomes top node, node A becomes C->right, and node D becomes A->left.
			If node C was balanced, A becomes left heavy, C becomes right heavy, and the subtree doesn't change height. Can return 0.
			If node C was left-heavy, A becomes balanced, C becomes balanced, and the subtree height is reduced by 1. Continue tree check.

			In the second case (C is right-heavy), D becomes top node, A becomes D->right, F becomes A->left, C becomes D->left, G becomes C->right.
			The tree height reduces by one, D is balanced, A is either balanced or right-heavy, C is either balanced or left heavy
			*/
			C = A->left;
			D = C->right;
			if (C->node_balance <= 0)
			{
				C->right = A;
				C->is_left = A->is_left;
				A->is_left = false;
				C->parent = A->parent;
				A->left = D;
				A->parent = C;
				if (*pp_top_node == A)
				{
					*pp_top_node = C;
				}
				else if (C->is_left)
				{
					C->parent->left = C;
				}
				else
				{
					C->parent->right = C;
				}

				if (D != nullptr)
				{
					D->is_left = true;
					D->parent = A;
				}
				if (C->node_balance == 0)
				{
					A->node_balance = -1;
					C->node_balance = 1;
					return 0;
				}

				A->node_balance = 0;
				C->node_balance = 0;
				// continue tree height check
				curr_node = C;
				continue;
			}
			else
			{
				/*
				In the second case (C is right-heavy), D becomes top node, A becomes D->right, F becomes A->left, C becomes D->left, G becomes C->right.
				The tree height reduces by one, D is balanced, A is either balanced or right-heavy, C is either balanced or left-heavy.
				*/
				node * F = D->right;
				node * G = D->left;
				D->left = C;
				D->right = A;

				A->left = F;
				C->right = G;

				A->node_balance = (D->node_balance < 0);
				C->node_balance = -(D->node_balance > 0);
				D->node_balance = 0;
				if (F != nullptr)
				{
					F->parent = A;
					F->is_left = true;
				}
				if (G != nullptr)
				{
					G->parent = C;
					G->is_left = false;
				}
				D->is_left = A->is_left;
				D->parent = A->parent;
				A->is_left = false;
				C->is_left = true;
				// fall through to the tail adjustment of the top node
			}
		}

		A->parent = D;
		C->parent = D;
		if (*pp_top_node == A)
		{
			*pp_top_node = D;
		}
		else if (D->is_left)
		{
			D->parent->left = D;
		}
		else
		{
			D->parent->right = D;
		}
		// continue tree height check
		curr_node = D;
	}
	// if we get here, we walked the tree all the way up, trying to find where its height didn't change.
	// We haven't found a node with unchanged height
	return true;   // tree height reduced
}

template<typename T, typename K, typename P, typename A> template<typename T1, typename K1, typename P1, typename A1>
typename avl_tree<T, K, P, A> & avl_tree<T, K, P, A>::operator=(avl_tree<T1, K1, P1, A1> const& src)
{
	// compare has to cast to void because tree types may be different
	if ((void const*)&src == (void*)this)
	{
		return *this;
	}

	reset();
	for (avl_tree<T1, K1, P1, A1>::const_iterator ii = src.cbegin(); ii != src.cend(); ii++)
	{
		insert_node(ii.key(), *ii);
	}

	return *this;
}

template<typename T, typename K, typename P, typename A>
typename avl_tree<T, K, P, A> & avl_tree<T, K, P, A>::operator=(avl_tree const& src)
{
	// compare has to cast to void because tree types may be different
	if (&src == this)
	{
		return *this;
	}

	reset();
	for (const_iterator ii = src.cbegin(); ii != src.cend(); ii++)
	{
		insert_node(ii.key(), *ii);
	}

	return *this;
}

template<typename T, typename K, typename P, typename A> void avl_tree<T, K, P, A>::validate_subtree(node const * tree, int * depth, int * number_nodes, node const** subtree_leftmost, node const** subtree_rightmost) const
{
	int left_depth, right_depth, left_nodes, right_nodes;
	node const* tmp_node = nullptr;

	node const* left = tree->left;
	if (left)
	{
		assert(_predicate(left->key, tree->key) < 0);
		assert(left->is_left);
		assert(left->parent == tree);
		validate_subtree(left, &left_depth, &left_nodes, subtree_leftmost, &tmp_node);
		// make sure that all keys in subtree are on the left of this node
		assert(_predicate(tmp_node->key, tree->key) < 0);
	}
	else
	{
		left_depth = 0;
		left_nodes = 0;
		*subtree_leftmost = tree;
	}
	node const* right = tree->right;
	if (right)
	{
		assert(_predicate(right->key, tree->key) > 0);
		assert(!right->is_left);
		assert(right->parent == tree);
		validate_subtree(right, &right_depth, &right_nodes, &tmp_node, subtree_rightmost);
		// make sure that all keys in subtree are on the right of this node
		assert(_predicate(tmp_node->key, tree->key) > 0);
	}
	else
	{
		right_depth = 0;
		right_nodes = 0;
		*subtree_rightmost = tree;
	}

	int node_balance = right_depth - left_depth;
	assert(node_balance == tree->node_balance);

	if (node_balance < 0)
	{
		assert(node_balance == -1);
		*depth = left_depth + 1;
	}
	else
	{
		assert(node_balance <= 1);
		*depth = right_depth + 1;
	}

	*number_nodes = 1 + left_nodes + right_nodes;
}

template<typename T, typename K, typename P, typename A> void avl_tree<T, K, P, A>::validate() const
{
	int depth = 0;
	int number_nodes = 0;
	node const* subtree_leftmost = static_cast<node const*>(&hdr);
	node const* subtree_rightmost = static_cast<node const*>(&hdr);
	if (hdr.root)
	{
		assert(hdr.root->parent == &hdr);
		validate_subtree(hdr.root, &depth, &number_nodes, &subtree_leftmost, &subtree_rightmost);
	}

	assert(depth == tree_height);
	assert(number_nodes == tree_count);
	assert(subtree_leftmost == hdr.leftmost);
	assert(subtree_rightmost == hdr.rightmost);
}

