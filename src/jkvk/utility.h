/*!
	@file		utility.h
	@author		Alastair Holmes
	@date		28/10/2018
 */

#ifndef JKVK_UTILITY_H
#define JKVK_UTILITY_H

#include <jkutil\assert.h>
#include <type_traits>
#include <vulkan\vulkan.hpp>

namespace jkvk
{

	//chains

	struct in_base_vulkan_structure_header
	{
		const vk::StructureType sType;
		const in_base_vulkan_structure_header* const pNext;
	};

	struct out_base_vulkan_structure_header
	{
		const vk::StructureType sType;
		out_base_vulkan_structure_header* const pNext;
	};

	using vulkan_chain_t = out_base_vulkan_structure_header*;
	using vulkan_const_chain_t = const in_base_vulkan_structure_header*;

	namespace _jkinternal
	{

		template <typename structureType, typename = int>
		struct has_pNext : std::false_type { };

		template <typename structureType>
		struct has_pNext<structureType, decltype((void)structureType::pNext, 0)> : std::true_type { };

	}

	//chain structure

	template <class structureType>
	using is_vulkan_chain_structure = typename jkvk::_jkinternal::has_pNext<structureType>::type;

	template <class structureType>
	using is_vulkan_chain_structure_v = is_vulkan_chain_structure<structureType>::value;

	template <class structureType>
	inline vk::StructureType get_vulkan_structure_type_value()
	{
		structureType structure = structureType();
		return reinterpret_cast<in_base_vulkan_structure_header&>(structure).sType;
	}

	//chain structure wrappers

	class base_vulkan_chain_structure_wrapper
	{};

	template <class wrappedStructureType>
	using is_vulkan_chain_structure_wrapper = std::bool_constant<std::is_base_of<base_vulkan_chain_structure_wrapper, wrappedStructureType>::value && !is_vulkan_chain_structure<wrappedStructureType>::value>::type;
	
	template <class wrappedStructureType>
	using is_vulkan_chain_structure_wrapper_v = is_vulkan_chain_structure_wrapper<wrappedStructureType>::value;

	namespace _jkinternal
	{

		template <class chainStructureType>
		class basic_vulkan_chain_structure_wrapper : public base_vulkan_chain_structure_wrapper
		{
		private:

			chainStructureType m_structure;

		public:

			static_assert(!is_vulkan_chain_structure_wrapper_v<chainStructureType> && is_vulkan_chain_structure_v<chainStructureType>);

			basic_vulkan_chain_structure_wrapper(const chainStructureType& p_structure)
				: m_structure(p_structure)
			{

			}

			vulkan_chain_t get_pointer()
			{
				return reinterpret_cast<vulkan_chain_t>(&m_structure);
			}

			vulkan_const_chain_t get_pointer() const
			{
				return reinterpret_cast<vulkan_const_chain_t>(&m_structure);
			}

			void set_next(std::nullptr_t)
			{
				m_structure.pNext = nullptr;
			}

			void set_next(vulkan_chain_t p_chain)
			{
				m_structure.pNext = p_chain;
			}

			void set_next(vulkan_const_chain_t p_chain)
			{
				static_assert(std::is_const_v<std::remove_pointer_t<decltype(m_structure.pNext)>>, "Cannot convert 'vulkan_const_chain_t' to 'vulkan_chain_t'.");
				if constexpr (std::is_const_v<std::remove_pointer_t<decltype(m_structure.pNext)>>)
				{
					m_structure.pNext = p_chain;
				}
			}

			vulkan_const_chain_t get_next() const
			{
				return m_structure.pNext;
			}

		};

		template <class chainStructureType>
		using make_wrapped_vulkan_chain_structure_type = std::conditional_t<is_vulkan_chain_structure_wrapper<chainStructureType>, chainStructureType, basic_vulkan_chain_structure_wrapper<chainStructureType>>;

		template <class chainStructureType>
		make_wrapped_vulkan_chain_structure_type<chainStructureType> make_wrapped_vulkan_chain_structure(const chainStructureType& p_structure)
		{
			return make_wrapped_vulkan_chain_structure_type<chainStructureType>(p_structure);
		}

	}

}

#endif