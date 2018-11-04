/*!
	@file		structure_chain.h
	@author		Alastair Holmes
	@date		28/10/2018
 */

#ifndef JKVK_STRUCTURE_CHAIN_H
#define JKVK_STRUCTURE_CHAIN_H

#include <vulkan\vulkan.hpp>
#include <jkvk\utility.h>
#include <jkutil\derived.h>

namespace jkvk
{

	namespace _jkinternal
	{

		template <class wrappedChainStructureType1, class wrappedChainStructureType2, class... wrappedChainStructureTypes>
		inline bool check_vulkan_chain(const wrappedChainStructureType1& p_structure1, const wrappedChainStructureType2& p_structure2, const wrappedChainStructureTypes&... p_structures)
		{
			static_assert(is_vulkan_chain_structure_wrapper_v<wrappedChainStructureType1> && is_vulkan_chain_structure_wrapper_v<wrappedChainStructureType2> && (is_vulkan_chain_structure_wrapper_v<wrappedChainStructureTypes> && ...));

			return (p_structure1.get_next() == p_structure2.get_pointer()) && _jkinternal::check_vulkan_chain(p_structure2, p_structures...);
		}

		template <class wrappedStructureType>
		inline bool check_vulkan_chain(const wrappedStructureType& p_structure)
		{
			static_assert(is_vulkan_chain_structure_wrapper_v<wrappedStructureType>);
			return (p_structure.get_next() == nullptr);
		}

		template <class... wrappedChainStructureTypes>
		inline bool check_vulkan_chain(const wrappedChainStructureTypes&... p_structures)
		{
			static_assert((is_vulkan_chain_structure_wrapper_v<wrappedChainStructureTypes> && ...));
			return true;
		}

		//-------------------//

		template <class... wrappedChainStructureTypes>
		inline void clear_vulkan_chain(wrappedChainStructureTypes&... p_structures)
		{
			static_assert((is_vulkan_chain_structure_wrapper_v<wrappedChainStructureTypes> && ...));

			(p_structures.set_next(nullptr), ...);
		}

		//-------------------//

		template <class wrappedChainStructureType1, class wrappedChainStructureType2, class... wrappedChainStructureTypes>
		inline void form_vulkan_chain(wrappedChainStructureType1& p_structure1, wrappedChainStructureType2& p_structure2, wrappedChainStructureTypes&... p_structures)
		{
			static_assert(is_vulkan_chain_structure_wrapper_v<wrappedChainStructureType1> && is_vulkan_chain_structure_wrapper_v<wrappedChainStructureType2> && (is_vulkan_chain_structure_wrapper_v<wrappedChainStructureTypes> && ...));

			//Stop user thinking they have passed in a chain
			//Stops implicit overwrite of a pre-existing chain
			JKUTIL_ASSERT(p_structure1.get_next() == nullptr);
			p_structure1.set_next(p_structure2.get_pointer());
			_jkinternal::form_vulkan_chain(p_structure2, p_structures...);
		}

		template <class wrappedStructureType>
		inline void form_vulkan_chain(wrappedStructureType& p_structure)
		{
			static_assert(is_vulkan_chain_structure_wrapper_v<wrappedStructureType>);

			//Stop user thinking they have passed in a chain
			//Stops implicit overwrite of a pre-existing chain
			JKUTIL_ASSERT(p_structure.get_next() == nullptr);
			p_structure.set_next(nullptr);
		}

		template <class... wrappedChainStructureTypes>
		inline void form_vulkan_chain(wrappedChainStructureTypes&... p_structures)
		{
			static_assert((is_vulkan_chain_structure_wrapper_v<wrappedChainStructureTypes> && ...));
		}

		//-------------------//

		template <class wrappedChainStructureType1, class wrappedChainStructureType2, class... wrappedChainStructureTypes>
		inline void form_vulkan_chain_overwrite(wrappedChainStructureType1& p_structure1, wrappedChainStructureType2& p_structure2, wrappedChainStructureTypes&... p_structures)
		{
			static_assert(is_vulkan_chain_structure_wrapper_v<wrappedChainStructureType1> && is_vulkan_chain_structure_wrapper_v<wrappedChainStructureType2> && (is_vulkan_chain_structure_wrapper_v<wrappedChainStructureTypes> && ...));
			p_structure1.set_next(p_structure2.get_pointer());
			_jkinternal::form_vulkan_chain_overwrite(p_structure2, p_structures...);
		}

		template <class wrappedStructureType>
		inline void form_vulkan_chain_overwrite(wrappedStructureType& p_structure)
		{
			static_assert(is_vulkan_chain_structure_wrapper_v<wrappedStructureType>);
			p_structure.set_next(nullptr);
		}

		template <class... wrappedChainStructureTypes>
		inline void form_vulkan_chain_overwrite(wrappedChainStructureTypes&... p_structures)
		{
			static_assert((is_vulkan_chain_structure_wrapper_v<wrappedChainStructureTypes> && ...));
		}

	}

	class base_structure_chain : public jkutil::derived_copyable_abstract_move_emplacer<base_structure_chain>, public jkutil::derived_copyable_abstract_copy_emplacer<base_structure_chain>
	{
	protected:

		template <class... chainStructureTypes>
		friend class structure_chain;

	public:

		virtual vulkan_chain_t get_chain() = 0;
		virtual vulkan_const_chain_t get_chain() const = 0;

		bool empty() const
		{
			return (get_chain() == nullptr);
		}

		template <class... chainStructureTypes>
		bool has_vulkan_structures() const
		{
			size_t found_types = 0;

			map([&return_value](vulkan_const_chain_t p_header)
			{
				const vk::StructureType type = p_header->sType;
				if (((type == get_vulkan_structure_type_value<chainStructureTypes>() || ...))
				{
					++found_types;
				}
			});

				return (found_types == sizeof...(chainStructureTypes));
		}

		template <class... chainStructureTypes, class callableType>
		void conditional_map(callableType&& p_callable)
		{
			map([callable = std::forward<callableType>(p_callable)](vulkan_chain_t p_header)
			{
				(conditional_call<chainStructureTypes>(p_header, std::forward<callableType>(callable))...);
			});
		}

		template <class... chainStructureTypes, class callableType>
		void conditional_map(callableType&& p_callable) const
		{
			map([callable = std::forward<callableType>(p_callable)](vulkan_const_chain_t p_header)
			{
				(conditional_call<chainStructureTypes>(p_header, std::forward<callableType>(callable))...);
			});
		}

	private:

		template <class chainStructureType, class callableType>
		static void conditional_call(vulkan_const_chain_t p_header, callableType&& p_callable)
		{
			static_assert(is_vulkan_chain_structure<chainStructureType>::value);

			if (p_header)
			{
				if (p_header->sType == get_vulkan_structure_type_value<chainStructureType>())
				{
					auto pre_next = p_header->pNext;
					std::forward<callableType>(p_callable)(*reinterpret_cast<const chainStructureType*>(p_header));
					JKUTIL_ASSERT(pre_next == p_header->pNext && "It is invalid to modify the pNext in a conditional map call.");
				}
			}
		}

		template <class chainStructureType, class callableType>
		static void conditional_call(vulkan_chain_t p_header, callableType&& p_callable)
		{
			static_assert(is_vulkan_chain_structure<chainStructureType>::value);

			if (p_header)
			{
				if (p_header->sType == get_vulkan_structure_type_value<chainStructureType>())
				{
					std::forward<callableType>(p_callable)(*reinterpret_cast<chainStructureType*>(p_header));
					JKUTIL_ASSERT(pre_next == p_header->pNext && "It is invalid to modify the pNext in a conditional map call.");
				}
			}
		}

		template <class callableType>
		void map(callableType&& p_callable) const
		{
			vulkan_const_chain_t chain = get_chain();

			while (chain)
			{
				std::forward<callableType>(p_callable)(chain);
				chain = chain->pNext;
			}

		}

		template <class callableType>
		void map(callableType&& p_callable)
		{
			vulkan_chain_t chain = get_chain();

			while (chain)
			{
				std::forward<callableType>(p_callable)(chain);
				chain = chain->pNext;
			}

		}

	};

	template <class... chainStructureTypes>
	class structure_chain : public base_structure_chain
	{
	public:

		structure_chain(const chainStructureTypes&... p_structures)
			: m_vulkan_structures(_jkinternal::make_wrapped_vulkan_chain_structure(p_structures)...)
		{
			form_vulkan_chain();
		}

		structure_chain(const structure_chain& p_instance)
			: m_vulkan_structures(p_instance.m_vulkan_structures)
		{
			form_vulkan_chain_overwrite();
		}

		structure_chain(structure_chain&& p_instance)
			: m_vulkan_structures(std::move(p_instance.m_vulkan_structures))
		{
			form_vulkan_chain_overwrite();
		}

		virtual vulkan_chain_t get_chain() override final
		{
			if constexpr (sizeof...(chainStructureTypes) == 0)
			{
				return nullptr;
			}
			else
			{
				return std::get<0>(m_vulkan_structures).get_pointer();
			}
		}

		virtual vulkan_const_chain_t get_chain() const override final
		{
			if constexpr (sizeof...(chainStructureTypes) == 0)
			{
				return nullptr;
			}
			else
			{
				return std::get<0>(m_vulkan_structures).get_pointer();
			}
		}

	private:

		virtual void clone_to(jkutil::derived_copyable<base_structure_chain, jkutil::allocator_pointer<jkutil::virtual_allocator>>& p_derived) const override final
		{
			if constexpr (sizeof...(chainStructureTypes) != 0)
			{
				p_derived.emplace<structure_chain>(*this);
			}
			else
			{
				p_derived.reset();
			}
		}

		virtual void move_to(jkutil::derived_copyable<base_structure_chain, jkutil::allocator_pointer<jkutil::virtual_allocator>>& p_derived) override final
		{
			if constexpr (sizeof...(chainStructureTypes) != 0)
			{
				p_derived.emplace<structure_chain>(std::move(*this));
			}
			else
			{
				p_derived.reset();
			}
		}

	private:

		void form_vulkan_chain()
		{
			form_vulkan_chain(std::make_index_sequence<sizeof...(chainStructureTypes)> {});
		}

		template <size_t... indexValues>
		void form_vulkan_chain(std::index_sequence<indexValues...>)
		{
			_jkinternal::form_vulkan_chain(std::get<indexValues>(m_vulkan_structures)...);
		}

		void form_vulkan_chain_overwrite()
		{
			form_vulkan_chain_overwrite(std::make_index_sequence<sizeof...(chainStructureTypes)> {});
		}

		template <size_t... indexValues>
		void form_vulkan_chain_overwrite(std::index_sequence<indexValues...>)
		{
			_jkinternal::form_vulkan_chain_overwrite(std::get<indexValues>(m_vulkan_structures)...);
		}

		std::tuple<_jkinternal::make_wrapped_vulkan_chain_structure_type<chainStructureTypes>...> m_vulkan_structures;

	};

	template <class... chainStructureTypes>
	structure_chain<chainStructureTypes...> make_chain(const chainStructureTypes&... p_structures)
	{
		return structure_chain<chainStructureTypes...>(p_structures...);
	}

	template <class... chainStructureTypes>
	structure_chain<chainStructureTypes&...> make_reference_chain(chainStructureTypes&... p_structure_references)
	{
		return structure_chain<chainStructureTypes&...>(p_structure_references...);
	}

	template <class allocatorType>
	class any_structure_chain : public base_structure_chain
	{
	private:

		template <class otherAllocatorType>
		friend class any_structure_chain;

	public:

		any_structure_chain(const allocatorType& p_allocator);

		any_structure_chain(const base_structure_chain& p_chain, const allocatorType& p_allocator);
		any_structure_chain(base_structure_chain&& p_chain, const allocatorType& p_allocator);

		any_structure_chain(const any_structure_chain&) = default;
		any_structure_chain(any_structure_chain&&) = default;

		virtual ~any_structure_chain() {};

		any_structure_chain& operator=(const any_structure_chain&) = default;
		any_structure_chain& operator=(any_structure_chain&&) = default;

		template <class otherAllocatorType>
		any_structure_chain& assign_value(const any_structure_chain<otherAllocatorType>& p_chain);

		template <class otherAllocatorType>
		any_structure_chain& assign_value(any_structure_chain<otherAllocatorType>&& p_chain);

		void swap(any_structure_chain& p_chain);

		template <class otherAllocatorType>
		void swap_value(any_structure_chain<otherAllocatorType>& p_chain);

		void reset();

		void emplace(const base_structure_chain& p_chain);
		void emplace(base_structure_chain&& p_chain);

		const allocatorType& get_allocator() const;

		virtual vulkan_chain_t get_chain() override final;
		virtual vulkan_const_chain_t get_chain() const override final;

	private:

		virtual void clone_to(jkutil::derived_copyable<base_structure_chain, jkutil::allocator_pointer<jkutil::virtual_allocator>>& p_derived) const override final;
		virtual void move_to(jkutil::derived_copyable<base_structure_chain, jkutil::allocator_pointer<jkutil::virtual_allocator>>& p_derived) override final;

	private:

		jkutil::virtual_allocator_adapter<allocatorType> get_abstract_allocator() const;

		jkutil::derived_copyable<base_structure_chain, allocatorType> m_chain;

	};

	template <class allocatorType>
	any_structure_chain<allocatorType>::any_structure_chain(const allocatorType& p_allocator)
		: m_chain(p_allocator)
	{

	}

	template <class allocatorType>
	any_structure_chain<allocatorType>::any_structure_chain(const base_structure_chain& p_chain, const allocatorType& p_allocator)
		: m_chain(p_allocator)
	{
		emplace(p_chain);
	}

	template <class allocatorType>
	any_structure_chain<allocatorType>::any_structure_chain(base_structure_chain&& p_chain, const allocatorType& p_allocator)
		: m_chain(p_allocator)
	{
		emplace(std::move(p_chain));
	}

	template<class allocatorType>
	template<class otherAllocatorType>
	inline auto any_structure_chain<allocatorType>::assign_value(const any_structure_chain<otherAllocatorType>& p_chain) -> any_structure_chain&
	{
		m_chain.assign_value(p_chain.m_chain);
		return *this;
	}

	template<class allocatorType>
	template<class otherAllocatorType>
	inline auto any_structure_chain<allocatorType>::assign_value(any_structure_chain<otherAllocatorType>&& p_chain) -> any_structure_chain&
	{
		m_chain.assign_value(std::move(p_chain.m_chain));
		return *this;
	}

	template<class allocatorType>
	template<class otherAllocatorType>
	inline void any_structure_chain<allocatorType>::swap_value(any_structure_chain<otherAllocatorType>& p_chain)
	{
		any_structure_chain<allocatorType> temp(get_allocator());
		temp.assign_value(p_chain);
		p_chain.assign_value(*this);
		*this.assign_value(temp);
	}

	template<class allocatorType>
	inline vulkan_chain_t any_structure_chain<allocatorType>::get_chain()
	{
		if (m_chain.has_value())
		{
			return m_chain.get()->get_chain();
		}
		else
		{
			return nullptr;
		}
	}

	template<class allocatorType>
	inline vulkan_const_chain_t any_structure_chain<allocatorType>::get_chain() const
	{
		if (m_chain.has_value())
		{
			return m_chain.get()->get_chain();
		}
		else
		{
			return nullptr;
		}
	}

	template <class allocatorType>
	void any_structure_chain<allocatorType>::clone_to(jkutil::derived_copyable<base_structure_chain, jkutil::allocator_pointer<jkutil::virtual_allocator>>& p_derived) const
	{
		p_derived.reset();
		if (m_chain.has_value())
		{
			m_chain.get()->clone(p_derived);
		}
	}

	template <class allocatorType>
	void any_structure_chain<allocatorType>::move_to(jkutil::derived_copyable<base_structure_chain, jkutil::allocator_pointer<jkutil::virtual_allocator>>& p_derived)
	{
		p_derived.reset();
		if (m_chain.has_value())
		{
			m_chain.get()->move(p_derived);
		}
	}

	template<class allocatorType>
	inline jkutil::virtual_allocator_adapter<allocatorType> any_structure_chain<allocatorType>::get_abstract_allocator() const
	{
		return jkutil::virtual_allocator_adapter<allocatorType>(m_chain.get_allocator());
	}

	template <class allocatorType>
	void any_structure_chain<allocatorType>::swap(any_structure_chain& p_chain)
	{
		m_chain.swap(p_chain.m_chain);
	}

	template <class allocatorType>
	void any_structure_chain<allocatorType>::reset()
	{
		m_chain.reset();
	}

	template<class allocatorType>
	inline void any_structure_chain<allocatorType>::emplace(const base_structure_chain& p_chain)
	{
		m_chain.abstract_emplace(p_chain);
	}

	template<class allocatorType>
	inline void any_structure_chain<allocatorType>::emplace(base_structure_chain&& p_chain)
	{
		m_chain.abstract_emplace(std::move(p_chain));
	}

	template<class allocatorType>
	inline const allocatorType& any_structure_chain<allocatorType>::get_allocator() const
	{
		return m_chain.get_allocator();
	}

}

#endif