/*!
	@file		info_structure_helpers.h
	@author		Alastair Holmes
	@date		03/11/2018
 */

#ifndef JKVK_INFO_STRUCTURE_HELPERS_H
#define JKVK_INFO_STRUCTURE_HELPERS_H

#include <jkvk\utility.h>
#include <jkvk\structure_chain.h>
#include <jkutil\assert.h>
#include <jkutil\vector.h>
#include <optional>
#include <variant>

namespace jkvk
{

	//In JKVK we wrap all of vulkan's 'info' structures in RAII helper classes that making setup and usage of these structures easier. This file contains
	//code to help wrap these 'info' structures.

	//These wrappers don't include top-level structure chains e.g. the get_info() for a given info structure, will output a info structure with pNext==nullptr.
	//But the chains for sub-info structures will be included. This design avoids un-needed usage of any_structure_chain's memory allocation.

	//The most basic wrapper; for info structures that don't need clever utilities. E.g. vk::PipelineInputAssemblyStateCreateInfo doesn't have any pointer elements and is just POD.

	namespace _jkinternal
	{

		template <class infoStructureType>
		infoStructureType add_chain(const infoStructureType& p_structure, vulkan_const_chain_t p_chain)
		{
			infoStructureType structure = p_structure;
			structure.pNext = p_chain;
			return structure;
		}

		template <class infoStructureType>
		infoStructureType add_chain(const infoStructureType& p_structure, vulkan_chain_t p_chain)
		{
			structureType structure = p_structure;
			structure.pNext = p_chain;
			return structure;
		}

		template <class infoStructureType>
		infoStructureType add_chain(const infoStructureType& p_structure, const base_structure_chain& p_chain)
		{
			static_assert(is_vulkan_chain_structure_v<infoStructureType>);

			infoStructureType structure = p_structure;
			structure.pNext = p_chain.get_chain();
			return structure;
		}

		template <class infoStructureType>
		infoStructureType add_chain(const infoStructureType& p_structure, base_structure_chain& p_chain)
		{
			static_assert(is_vulkan_chain_structure_v<infoStructureType>);

			infoStructureType structure = p_structure;
			structure.pNext = p_chain.get_chain();
			return structure;
		}

	}

	template <class infoStructureType>
	class info_structure_adapter
	{
	public:

		using info_structure_type = infoStructureType;

		template <class otherAllocatorType>
		using rebind = info_structure_adapter<infoStructureType>;

		info_structure_adapter(const infoStructureType& p_structure = infoStructureType());

		//rebind copy constructor
		template <class otherAllocatorType>
		info_structure_adapter(const info_structure_adapter<infoStructureType>& p_instance, const otherAllocatorType& p_allocator);

		//rebind move constructor
		template <class otherAllocatorType>
		info_structure_adapter(info_structure_adapter<infoStructureType>&& p_instance, const otherAllocatorType& p_allocator);

		info_structure_adapter(const info_structure_adapter&) = default;
		info_structure_adapter(info_structure_adapter&&) = default;

		info_structure_adapter& operator=(const info_structure_adapter&) = default;
		info_structure_adapter& operator=(info_structure_adapter&&) = default;

		infoStructureType get_info() const;

	private:

		infoStructureType m_structure;

	};

	template<class infoStructureType>
	inline info_structure_adapter<infoStructureType>::info_structure_adapter(const infoStructureType& p_structure)
		: m_structure(p_structure)
	{
		if constexpr (is_vulkan_chain_structure_v<infoStructureType>)
		{
			JKUTIL_ASSERT(m_structure.pNext == nullptr);
		}
	}

	template<class infoStructureType>
	template<class otherAllocatorType>
	inline info_structure_adapter<infoStructureType>::info_structure_adapter(const info_structure_adapter<infoStructureType>& p_instance, const otherAllocatorType& p_allocator)
		: m_structure(p_instance.m_structure)
	{
	}

	template<class infoStructureType>
	template<class otherAllocatorType>
	inline info_structure_adapter<infoStructureType>::info_structure_adapter(info_structure_adapter<infoStructureType>&& p_instance, const otherAllocatorType& p_allocator)
		: m_structure(std::move(p_instance.m_structure))
	{
	}

	template<class infoStructureType>
	inline infoStructureType info_structure_adapter<infoStructureType>::get_info() const
	{
		return m_structure;
	}

	//A helper for other wrappers to store sub info structures that have structure chains. 
	template <class infoStructureWrapper, class allocatorType>
	class sub_chain_structure_wrapper
	{
	public:

		using info_structure_type = typename infoStructureWrapper::info_structure_type;

		template <class otherAllocatorType>
		using rebind = sub_chain_structure_wrapper<typename infoStructureWrapper::rebind<otherAllocatorType>, otherAllocatorType>;

		sub_chain_structure_wrapper(const infoStructureWrapper& p_structure, const allocatorType& p_allocator, const base_structure_chain& p_chain);
		sub_chain_structure_wrapper(infoStructureWrapper&& p_structure, const allocatorType& p_allocator, const base_structure_chain& p_chain);

		//rebind copy constructors
		template <class otherAllocatorType>
		sub_chain_structure_wrapper(const rebind<otherAllocatorType>& p_helper, const allocatorType& p_allocator);

		//rebind move constructor
		template <class otherAllocatorType>
		sub_chain_structure_wrapper(rebind<otherAllocatorType>&& p_helper, const allocatorType& p_allocator);

		sub_chain_structure_wrapper(const sub_chain_structure_wrapper&) = default;
		sub_chain_structure_wrapper(sub_chain_structure_wrapper&&) = default;

		sub_chain_structure_wrapper& operator=(const sub_chain_structure_wrapper&) = default;
		sub_chain_structure_wrapper& operator=(sub_chain_structure_wrapper&&) = default;

		info_structure_type get_info() const;

		const infoStructureWrapper& get_wrapper() const;
		infoStructureWrapper& get_wrapper();

		const base_structure_chain& get_chain() const;
		base_structure_chain& get_chain();

	private:

		any_structure_chain<allocatorType> m_chain;
		infoStructureWrapper m_wrapper;

	};

	template<class infoStructureWrapper, class allocatorType>
	inline sub_chain_structure_wrapper<infoStructureWrapper, allocatorType>::sub_chain_structure_wrapper(const infoStructureWrapper& p_structure, const allocatorType & p_allocator, const base_structure_chain & p_chain)
		: m_chain(p_chain, p_allocator),
		m_wrapper(p_structure)
	{
	}

	template<class infoStructureWrapper, class allocatorType>
	inline sub_chain_structure_wrapper<infoStructureWrapper, allocatorType>::sub_chain_structure_wrapper(infoStructureWrapper&& p_structure, const allocatorType & p_allocator, const base_structure_chain& p_chain)
		: m_chain(p_chain, p_allocator),
		m_wrapper(std::move(p_structure))
	{
	}

	template<class infoStructureWrapper, class allocatorType>
	template<class otherAllocatorType>
	inline sub_chain_structure_wrapper<infoStructureWrapper, allocatorType>::sub_chain_structure_wrapper(const rebind<otherAllocatorType>& p_helper, const allocatorType & p_allocator)
		: m_chain(p_helper.m_chain, p_allocator),
		m_wrapper(p_helper.m_wrapper, p_allocator)
	{
	}

	template<class infoStructureWrapper, class allocatorType>
	template<class otherAllocatorType>
	inline sub_chain_structure_wrapper<infoStructureWrapper, allocatorType>::sub_chain_structure_wrapper(rebind<otherAllocatorType>&& p_helper, const allocatorType & p_allocator)
		: m_chain(std::move(p_helper.m_chain), p_allocator),
		m_wrapper(std::move(p_helper.m_wrapper), p_allocator)
	{

	}

	template<class infoStructureWrapper, class allocatorType>
	inline const infoStructureWrapper& sub_chain_structure_wrapper<infoStructureWrapper, allocatorType>::get_wrapper() const
	{
		return m_wrapper;
	}

	template<class infoStructureWrapper, class allocatorType>
	inline infoStructureWrapper& sub_chain_structure_wrapper<infoStructureWrapper, allocatorType>::get_wrapper()
	{
		return m_wrapper;
	}

	template<class infoStructureWrapper, class allocatorType>
	inline const base_structure_chain& sub_chain_structure_wrapper<infoStructureWrapper, allocatorType>::get_chain() const
	{
		return m_chain;
	}

	template<class infoStructureWrapper, class allocatorType>
	inline base_structure_chain& sub_chain_structure_wrapper<infoStructureWrapper, allocatorType>::get_chain()
	{
		return m_chain;
	}

	template<class infoStructureWrapper, class allocatorType>
	inline auto sub_chain_structure_wrapper<infoStructureWrapper, allocatorType>::get_info() const -> info_structure_type
	{
		return _jkinternal::add_chain(m_wrapper.get_info(), m_chain);
	}

	template <class infoStructureWrapper>
	class optional_info_structure_wrapper
	{
	private:

		using internal_info_structure_type = typename infoStructureWrapper::info_structure_type;

	public:

		using info_structure_type = const internal_info_structure_type*;

		template <class otherAllocatorType>
		using rebind = optional_info_structure_wrapper<typename infoStructureWrapper::rebind<otherAllocatorType>>;

		optional_info_structure_wrapper();
		optional_info_structure_wrapper(const infoStructureWrapper& p_wrapper);
		optional_info_structure_wrapper(infoStructureWrapper&& p_wrapper);

		template <class allocatorType, class otherAllocatorType>
		optional_info_structure_wrapper(const rebind<otherAllocatorType>& p_instance, const allocatorType& p_allocator);

		template <class allocatorType, class otherAllocatorType>
		optional_info_structure_wrapper(rebind<otherAllocatorType>&& p_instance, const allocatorType& p_allocator);

		optional_info_structure_wrapper(const optional_info_structure_wrapper&) = default;
		optional_info_structure_wrapper(optional_info_structure_wrapper&&) = default;

		optional_info_structure_wrapper& operator=(const optional_info_structure_wrapper&) = default;
		optional_info_structure_wrapper& operator=(optional_info_structure_wrapper&&) = default;

		template <class... argumentTypes>
		void emplace(argumentTypes&&... p_arguments);
		void reset();

		bool has_value() const;

		info_structure_type get_info() const;

	private:

		std::optional<infoStructureWrapper> m_wrapper;

	private:

		mutable typename internal_info_structure_type m_structure;

	};

	template<class infoStructureWrapper>
	inline optional_info_structure_wrapper<infoStructureWrapper>::optional_info_structure_wrapper()
	{
	}

	template<class infoStructureWrapper>
	inline optional_info_structure_wrapper<infoStructureWrapper>::optional_info_structure_wrapper(const infoStructureWrapper & p_wrapper)
		: m_wrapper(p_wrapper)
	{
	}

	template<class infoStructureWrapper>
	inline optional_info_structure_wrapper<infoStructureWrapper>::optional_info_structure_wrapper(infoStructureWrapper && p_wrapper)
		: m_wrapper(std::move(p_wrapper))
	{
	}

	template<class infoStructureWrapper>
	template <class allocatorType, class otherAllocatorType>
	inline optional_info_structure_wrapper<infoStructureWrapper>::optional_info_structure_wrapper(const rebind<otherAllocatorType>& p_instance, const allocatorType& p_allocator)
	{
		m_wrapper.emplace(p_instance.m_wrapper, p_allocator);
	}

	template<class infoStructureWrapper>
	template <class allocatorType, class otherAllocatorType>
	inline optional_info_structure_wrapper<infoStructureWrapper>::optional_info_structure_wrapper(rebind<otherAllocatorType>&& p_instance, const allocatorType & p_allocator)
	{
		m_wrapper.emplace(std::move(p_instance.m_wrapper), p_allocator);
	}

	template<class infoStructureWrapper>
	template<class... argumentTypes>
	inline void optional_info_structure_wrapper<infoStructureWrapper>::emplace(argumentTypes&&... p_arguments)
	{
		m_wrapper.emplace(std::forward<argumentTypes>(p_arguments)...);
	}

	template<class infoStructureWrapper>
	inline void optional_info_structure_wrapper<infoStructureWrapper>::reset()
	{
		m_wrapper.reset();
	}

	template<class infoStructureWrapper>
	inline bool optional_info_structure_wrapper<infoStructureWrapper>::has_value() const
	{
		return m_wrapper.has_value();
	}

	template<class infoStructureWrapper>
	inline auto optional_info_structure_wrapper<infoStructureWrapper>::get_info() const -> info_structure_type
	{
		if (has_value())
		{
			m_structure = m_wrapper.value().get_info();
			return &m_structure;
		}
		else
		{
			m_structure = internal_info_structure_type();
			return nullptr;
		}
	}

	template <class infoStructureWrapper, class allocatorType>
	class info_structure_array_wrapper
	{
	private:

		using internal_info_structure_type = typename infoStructureWrapper::info_structure_type;

	public:

		using info_structure_type = const internal_info_structure_type*;

		template <class otherAllocatorType>
		using rebind = info_structure_array_wrapper<typename infoStructureWrapper::rebind<otherAllocatorType>, otherAllocatorType>;

		info_structure_array_wrapper(const allocatorType& p_allocator);

		template <class otherAllocatorType>
		info_structure_array_wrapper(const rebind<otherAllocatorType>& p_instance, const allocatorType& p_allocator);

		template <class otherAllocatorType>
		info_structure_array_wrapper(rebind<otherAllocatorType>&& p_instance, const allocatorType& p_allocator);

		void push_back(const infoStructureWrapper& p_wrapper);

		void push_back(infoStructureWrapper&& p_wrapper);

		template <class... argumentTypes>
		infoStructureWrapper& emplace_back(argumentTypes&&... p_arguments);

		void reserve(size_t p_count);

		size_t size() const;
		bool empty() const;
		void clear();

		info_structure_type get_info() const;

	private:

		jkutil::vector<infoStructureWrapper, allocatorType> m_wrappers;

	private:

		mutable jkutil::vector<internal_info_structure_type, allocatorType> m_info_structures;

	};

	template<class infoStructureWrapper, class allocatorType>
	inline info_structure_array_wrapper<infoStructureWrapper, allocatorType>::info_structure_array_wrapper(const allocatorType& p_allocator)
		: m_wrappers(p_allocator), m_info_structures(p_allocator)
	{
	}

	template<class infoStructureWrapper, class allocatorType>
	template<class otherAllocatorType>
	inline info_structure_array_wrapper<infoStructureWrapper, allocatorType>::info_structure_array_wrapper(const rebind<otherAllocatorType>& p_instance, const allocatorType & p_allocator)
		: info_structure_array_wrapper(p_allocator)
	{
		m_wrappers.reserve(p_instance.size());
		for (const auto& wrapper : p_instance.m_wrappers)
		{
			m_wrappers.push_back(wrapper);
		}
	}

	template<class infoStructureWrapper, class allocatorType>
	template<class otherAllocatorType>
	inline info_structure_array_wrapper<infoStructureWrapper, allocatorType>::info_structure_array_wrapper(rebind<otherAllocatorType>&& p_instance, const allocatorType & p_allocator)
		: info_structure_array_wrapper(p_allocator)
	{
		m_wrappers.reserve(p_instance.size());
		for (auto& wrapper : p_instance.m_wrappers)
		{
			m_wrappers.push_back(std::move(wrapper));
		}
	}

	template<class infoStructureWrapper, class allocatorType>
	inline void info_structure_array_wrapper<infoStructureWrapper, allocatorType>::push_back(const infoStructureWrapper & p_wrapper)
	{
		m_wrappers.push_back(p_wrapper);
	}

	template<class infoStructureWrapper, class allocatorType>
	inline void info_structure_array_wrapper<infoStructureWrapper, allocatorType>::push_back(infoStructureWrapper && p_wrapper)
	{
		m_wrappers.push_back(std::move(p_wrapper));
	}

	template<class infoStructureWrapper, class allocatorType>
	template<class ...argumentTypes>
	inline infoStructureWrapper & info_structure_array_wrapper<infoStructureWrapper, allocatorType>::emplace_back(argumentTypes && ...p_arguments)
	{
		return m_wrappers.emplace_back(std::forward<argumentTypes>(p_arguments)...);
	}

	template<class infoStructureWrapper, class allocatorType>
	inline void info_structure_array_wrapper<infoStructureWrapper, allocatorType>::reserve(size_t p_count)
	{
		m_wrappers.reserve(p_count);
	}

	template<class infoStructureWrapper, class allocatorType>
	inline size_t info_structure_array_wrapper<infoStructureWrapper, allocatorType>::size() const
	{
		return m_wrappers.size();
	}

	template<class infoStructureWrapper, class allocatorType>
	inline bool info_structure_array_wrapper<infoStructureWrapper, allocatorType>::empty() const
	{
		return m_wrappers.empty();
	}

	template<class infoStructureWrapper, class allocatorType>
	inline void info_structure_array_wrapper<infoStructureWrapper, allocatorType>::clear()
	{
		m_wrappers.clear();
	}

	template<class infoStructureWrapper, class allocatorType>
	inline auto info_structure_array_wrapper<infoStructureWrapper, allocatorType>::get_info() const -> info_structure_type
	{
		m_info_structures.clear();
		m_info_structures.reserve(m_wrappers.size());

		for (const auto& wrapper : m_wrappers)
		{
			m_info_structures.push_back(wrapper.get_info());
		}

		return m_info_structures.data();
	}

	template <class... infoStructureWrappers>
	class info_structure_variant_wrapper
	{
	public:

		static_assert(jkutil::all_same<typename infoStructureWrappers::info_structure_type...>::value, "The wrapped type of info structure must be all the same for all the wrappers in a variant.");

		using info_structure_type = typename std::common_type<typename infoStructureWrappers::info_structure_type...>::type;

		template <class otherAllocatorType>
		using rebind = info_structure_variant_wrapper<typename infoStructureWrappers::rebind<otherAllocatorType>...>;

		template <class infoStructureWrapper, class... argumentTypes>
		info_structure_variant_wrapper(std::in_place_type_t<infoStructureWrapper>, argumentTypes&&... p_arguments);

		template <class allocatorType, class otherAllocatorType>
		info_structure_variant_wrapper(const rebind<otherAllocatorType>& p_instance, const allocatorType& p_allocator);

		template <class allocatorType, class otherAllocatorType>
		info_structure_variant_wrapper(rebind<otherAllocatorType>&& p_instance, const allocatorType& p_allocator);

		info_structure_variant_wrapper(const info_structure_variant_wrapper&) = default;
		info_structure_variant_wrapper(info_structure_variant_wrapper&&) = default;

		info_structure_variant_wrapper& operator=(const info_structure_variant_wrapper&) = default;
		info_structure_variant_wrapper& operator=(info_structure_variant_wrapper&&) = default;

		const std::variant<infoStructureWrappers...>& get_variant() const;
		std::variant<infoStructureWrappers...>& get_variant();

		info_structure_type get_info() const;

	private:

		template <class otherAllocatorType>
		using variant_rebind = std::variant<typename infoStructureWrappers::rebind<otherAllocatorType>...>;

		template <class otherAllocatorType>
		variant_rebind<otherAllocatorType> copy_rebind(const otherAllocatorType& p_allocator) const;

		template <class otherAllocatorType>
		variant_rebind<otherAllocatorType> move_rebind(const otherAllocatorType& p_allocator);

		std::variant<infoStructureWrappers...> m_variant;

	};

	template<class... infoStructureWrappers>
	template<class infoStructureWrapper, class ...argumentTypes>
	inline info_structure_variant_wrapper<infoStructureWrappers...>::info_structure_variant_wrapper(std::in_place_type_t<infoStructureWrapper> p_type, argumentTypes&&... p_arguments)
		: m_variant(p_type, std::forward<argumentTypes>(p_arguments)...)
	{
	}

	template<class ...infoStructureWrappers>
	template<class allocatorType, class otherAllocatorType>
	inline info_structure_variant_wrapper<infoStructureWrappers...>::info_structure_variant_wrapper(const rebind<otherAllocatorType>& p_instance, const allocatorType & p_allocator)
		: m_variant(p_instance.copy_rebind(p_allocator))
	{
	}

	template<class ...infoStructureWrappers>
	template<class allocatorType, class otherAllocatorType>
	inline info_structure_variant_wrapper<infoStructureWrappers...>::info_structure_variant_wrapper(rebind<otherAllocatorType>&& p_instance, const allocatorType & p_allocator)
		: m_variant(p_instance.move_rebind(p_allocator))
	{
	}

	template<class ...infoStructureWrappers>
	template<class otherAllocatorType>
	inline auto info_structure_variant_wrapper<infoStructureWrappers...>::copy_rebind(const otherAllocatorType& p_allocator) const -> variant_rebind<otherAllocatorType>
	{
		return std::visit([](const auto& p_wrapper)
		{
			return variant_rebind<otherAllocatorType>(std::in_place_type<typename std::remove_reference_t<decltype(p_wrapper)>::rebind<otherAllocatorType>>, p_wrapper, p_allocator);
		},
		m_variant);
	}

	template<class ...infoStructureWrappers>
	template<class otherAllocatorType>
	inline auto info_structure_variant_wrapper<infoStructureWrappers...>::move_rebind(const otherAllocatorType& p_allocator) -> variant_rebind<otherAllocatorType>
	{
		return std::visit([](auto& p_wrapper)
		{
			return variant_rebind<otherAllocatorType>(std::in_place_type<typename std::remove_reference_t<decltype(p_wrapper)>::rebind<otherAllocatorType>>, std::move(p_wrapper), p_allocator);
		},
		m_variant);
	}

	template<class... infoStructureWrappers>
	inline const std::variant<infoStructureWrappers...>& info_structure_variant_wrapper<infoStructureWrappers...>::get_variant() const
	{
		return m_variant;
	}

	template<class... infoStructureWrappers>
	inline std::variant<infoStructureWrappers...>& info_structure_variant_wrapper<infoStructureWrappers...>::get_variant()
	{
		return m_variant;
	}

	template<class... infoStructureWrappers>
	inline auto info_structure_variant_wrapper<infoStructureWrappers...>::get_info() const -> info_structure_type
	{
		return std::visit(
			[](const auto& p_object) -> typename info_structure_type
			{
				return p_object.get_info();
			},
			m_variant);
	}

}

#endif