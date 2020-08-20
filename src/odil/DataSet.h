/*************************************************************************
 * odil - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _8424446e_1153_4acc_9f57_e86faa7246e3
#define _8424446e_1153_4acc_9f57_e86faa7246e3

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <string>
#include <vector>

#include "odil/Element.h"
#include "odil/odil.h"
#include "odil/Value.h"

namespace odil
{

/**
 * @brief DICOM Data set.
 */
class ODIL_API DataSet
{
public:
    /// @brief Create an empty data set.
    explicit DataSet(std::string const & transfer_syntax="");

    /** @addtogroup default_operations Default class operations
     * @{
     */
    ~DataSet() =default;
    DataSet(DataSet const &) =default;
    DataSet(DataSet &&) =default;
    DataSet & operator=(DataSet const &) =default;
    DataSet & operator=(DataSet &&) =default;
    /// @}

    /// @brief Add an element to the dataset.
    void add(Tag const & tag, Element const & element);

    /// @brief Add an element to the dataset.
    void add(Tag const & tag, Element && element);

    /// @brief Add an empty element to the dataset.
    void add(Tag const & tag, VR vr=VR::UNKNOWN);

#define ODIL_DATASET_ADD(type) \
    void add(\
        Tag const & tag, Value::type const & value, VR vr=VR::UNKNOWN);\
    void add(\
        Tag const & tag, Value::type && value, VR vr=VR::UNKNOWN); \
    void add(\
        Tag const & tag, \
        std::initializer_list<Value::type::value_type> const & value, \
        VR vr=VR::UNKNOWN);
    /*
     * No need for for a rvalue reference version of std::initializer_list:
     * copying a std::initializer_list does not copy the underlying objects.
     */

    ODIL_DATASET_ADD(Integers)
    ODIL_DATASET_ADD(Reals)
    ODIL_DATASET_ADD(Strings)
    ODIL_DATASET_ADD(DataSets)
    ODIL_DATASET_ADD(Binary)
#undef ODIL_DATASET_ADD

    /// @brief Add an element to the dataset.
    void add(
        Tag const & tag, std::initializer_list<int> const & value,
        VR vr=VR::UNKNOWN);

    /// @brief Add an element to the dataset.
    void add(
        Tag const & tag,
        std::initializer_list<std::initializer_list<uint8_t>> const & value,
        VR vr=VR::UNKNOWN);

    /**
     * @brief Remove an element from the data set.
     *
     * If the element is not in the data set, a odil::Exception is raised.
     */
    void remove(Tag const & tag);

    /// @brief Test whether the data set is empty.
    bool empty() const;

    /// @brief Return the number of elements in the data set.
    std::size_t size() const;

    /// @brief Test whether an element is in the data set.
    bool has(Tag const & tag) const;

    /**
     * @brief Return the VR of an element in the data set.
     *
     * If the element is not in the data set, a odil::Exception is raised.
     */
    VR get_vr(Tag const & tag) const;

    /**
     * @brief Test whether an element of the data set is empty.
     *
     * If the element is not in the data set, a odil::Exception is raised.
     */
    bool empty(Tag const & tag) const;

    /**
     * @brief Return the number of values in an element of the data set.
     *
     * If the element is not in the data set, a odil::Exception is raised.
     */
    std::size_t size(Tag const & tag) const;

    /**
     * @brief Access the given element.
     *
     * If the element is not in the data set, a odil::Exception is raised.
     */
    Element const & operator[](Tag const & tag) const;

    /**
     * @brief Access the given element.
     *
     * If the element is not in the data set, a odil::Exception is raised.
     */
    Element & operator[](Tag const & tag);

    /// @brief Test whether an existing element has integer type.
    bool is_int(Tag const & tag) const;

    /// @brief Return the integers contained in an existing element (read-only).
    Value::Integers const & as_int(Tag const & tag) const;

    /// @brief Return the integers contained in an existing element (read-write).
    Value::Integers & as_int(Tag const & tag);

    /// @brief Return an integer contained in an existing element (read-only).
    Value::Integer const & as_int(Tag const & tag, unsigned int position) const;

    /// @brief Test whether an existing element has real type.
    bool is_real(Tag const & tag) const;

    /// @brief Return the reals contained in an existing element (read-only).
    Value::Reals const & as_real(Tag const & tag) const;

    /// @brief Return the reals contained in an existing element (read-write).
    Value::Reals & as_real(Tag const & tag);

    /// @brief Return an real contained in an existing element (read-only).
    Value::Real const & as_real(Tag const & tag, unsigned int position) const;

    /// @brief Test whether an existing element has string type.
    bool is_string(Tag const & tag) const;

    /// @brief Return the strings contained in an existing element (read-only).
    Value::Strings const & as_string(Tag const & tag) const;

    /// @brief Return the strings contained in an existing element (read-write).
    Value::Strings & as_string(Tag const & tag);

    /// @brief Return a string contained in an existing element (read-only).
    Value::String const & as_string(Tag const & tag, unsigned int position) const;

    /// @brief Test whether an existing element has data set type.
    bool is_data_set(Tag const & tag) const;

    /// @brief Return the data sets contained in an existing element (read-only).
    Value::DataSets const & as_data_set(Tag const & tag) const;

    /// @brief Return the data sets contained in an existing element (read-write).
    Value::DataSets & as_data_set(Tag const & tag);

    /// @brief Return a data set contained in an existing element (read-only).
    std::shared_ptr<DataSet> const &
    as_data_set(Tag const & tag, unsigned int position) const;

    /// @brief Test whether an existing element has binary type.
    bool is_binary(Tag const & tag) const;

    /// @brief Return the binary items contained in an existing element (read-only).
    Value::Binary const & as_binary(Tag const & tag) const;

    /// @brief Return the binary items contained in an existing element (read-write).
    Value::Binary & as_binary(Tag const & tag);

    /// @brief Return a binary item contained in an existing element (read-only).
    Value::Binary::value_type const &
    as_binary(Tag const & tag, unsigned int position) const;

    /// @brief Iterator to the elements.
    typedef std::map<Tag, Element>::const_iterator const_iterator;

    /// @brief Return an iterator to the start of the elements.
    const_iterator begin() const;

    /// @brief Return an iterator to the end of the elements.
    const_iterator end() const;

    /// @brief Equality test.
    bool operator==(DataSet const & other) const;

    /// @brief Difference test.
    bool operator!=(DataSet const & other) const;

    /**
     * @brief Clear the data set (data_set.empty() will be true). All iterators
     * and all references to elements will be invalidated.
     */
    void clear();

    /// @brief Clear the element (data_set.empty(tag) will be true).
    void clear(Tag const & tag);

    /// @brief Return the current transfer syntax.
    std::string const & get_transfer_syntax() const;

    /// @brief Set the current transfer syntax.
    void set_transfer_syntax(std::string const & transfer_syntax);

private:
    typedef std::map<Tag, Element> ElementMap;

    ElementMap _elements;

    /// @brief Current transfer syntax.
    std::string _transfer_syntax;
};

}

#endif // _8424446e_1153_4acc_9f57_e86faa7246e3
