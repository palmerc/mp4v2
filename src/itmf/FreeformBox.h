///////////////////////////////////////////////////////////////////////////////
//
//  The contents of this file are subject to the Mozilla Public License
//  Version 1.1 (the "License"); you may not use this file except in
//  compliance with the License. You may obtain a copy of the License at
//  http://www.mozilla.org/MPL/
//
//  Software distributed under the License is distributed on an "AS IS"
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
//  License for the specific language governing rights and limitations
//  under the License.
// 
//  The Original Code is MP4v2.
// 
//  The Initial Developer of the Original Code is Kona Blend.
//  Portions created by Kona Blend are Copyright (C) 2008.
//  All Rights Reserved.
//
//  Contributors:
//      Kona Blend, kona8lend@@gmail.com
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MP4V2_IMPL_ITMF_FREEFORMBOX_H
#define MP4V2_IMPL_ITMF_FREEFORMBOX_H

namespace mp4v2 { namespace impl { namespace itmf {

///////////////////////////////////////////////////////////////////////////////

/// Functional class for freeform-box (Cover-art Box) support.
///
class MP4V2_EXPORT FreeformBox
{
public:
    /// Data object for freeform-box item.
    /// This object correlates to one freeform->data atom and offers automatic
    /// memory freeing when <b>autofree</b> is true.
    ///
    class MP4V2_EXPORT Item
    {
    public:
        Item();
        Item( const Item& );
        ~Item();

        Item& operator=( const Item& );

        /// Reset to state of newly constructed object.
        /// If <b>buffer</b> is not NULL and <b>autofree</b> is true the
        /// buffer will be free'd.
        void reset();

        string    mean;
        string    name;
        BasicType type;     ///< freeform-box type.
        uint8_t*  buffer;   ///< buffer point to raw freeform-box data.
        uint32_t  size;     ///< size of freeform-box buffer size in bytes.
        bool      autofree; ///< when true invoke free(buffer) upon destruction.
    };

    /// Object representing a list of freeform-box items.
    typedef vector<Item> ItemList;

    /// Fetch list of freeform-box items from file.
    ///
    /// @param hFile on which to operate.
    /// @param out vector of ArtItem objects.
    ///
    /// @return <b>true</b> on failure, <b>false</b> on success.
    ///
    static bool list( MP4FileHandle hFile, ItemList& out );

    /// Add freeform-box item to file.
    /// Any necessary metadata atoms are first created.
    /// Additionally, if an empty data-atom exists it will be used,
    /// otherwise a new data-atom is added to <b>freeform-atom</b>.
    ///
    /// @param hFile on which to operate.
    /// @param item freeform-box object to place in file.
    ///
    /// @return <b>true</b> on failure, <b>false</b> on success.
    ///
    static bool add( MP4FileHandle hFile, const Item& item );

    /// Replace freeform-box item in file.
    ///
    /// @param hFile on which to operate.
    /// @param item freeform-box object to place in file.
    /// @param index 0-based index of image to replace.
    ///
    /// @return <b>true</b> on failure, <b>false</b> on success.
    ///
    static bool set( MP4FileHandle hFile, const Item& item, uint32_t index );

    /// Fetch freeform-box item from file.
    ///
    /// @param hFile on which to operate.
    /// @param item freeform-box object populated with data.
    ///     The resulting object owns the malloc'd buffer and <b>item.autofree</b>
    ///     is set to true for convenient memory management.
    /// @param index 0-based index of image to fetch.
    ///
    /// @return <b>true</b> on failure, <b>false</b> on success.
    ///
    static bool get( MP4FileHandle hFile, Item& item, uint32_t index );

    /// Remove freeform-box item from file.
    ///
    /// @param hFile on which to operate.
    /// @param index 0-based index of image to remove.
    ///     Default value indicates wildcard behavior to remove all items.
    ///
    /// @return <b>true</b> on failure, <b>false</b> on success.
    ///
    static bool remove( MP4FileHandle hFile, uint32_t index = numeric_limits<uint32_t>::max() );
};

///////////////////////////////////////////////////////////////////////////////

}}} // namespace mp4v2::impl::itmf

#endif // MP4V2_IMPL_ITMF_FREEFORMBOX_H
