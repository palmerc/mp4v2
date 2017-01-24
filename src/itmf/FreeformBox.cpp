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

#include "impl.h"

namespace mp4v2 { namespace impl { namespace itmf {

///////////////////////////////////////////////////////////////////////////////

FreeformBox::Item::Item()
    : mean     ( "" )
    , name     ( "" )
    , type     ( BT_UNDEFINED )
    , buffer   ( NULL )
    , size     ( 0 )
    , autofree ( false )
{
}

///////////////////////////////////////////////////////////////////////////////

FreeformBox::Item::Item( const Item& rhs )
    : mean     ( "" )
    , name     ( "" )
    , type     ( BT_UNDEFINED )
    , buffer   ( NULL )
    , size     ( 0 )
    , autofree ( false )
{
    operator=( rhs );
}

///////////////////////////////////////////////////////////////////////////////

FreeformBox::Item::~Item()
{
    reset();
}

///////////////////////////////////////////////////////////////////////////////

FreeformBox::Item&
FreeformBox::Item::operator=( const Item& rhs )
{
    mean     = rhs.mean;
    name     = rhs.name;
    type     = rhs.type;
    size     = rhs.size;
    autofree = rhs.autofree;

    if( rhs.autofree ) {
        buffer = (uint8_t*)MP4Malloc(rhs.size);
        memcpy( buffer, rhs.buffer, rhs.size );
    }
    else {
        buffer = rhs.buffer;
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

void
FreeformBox::Item::reset()
{
    if( autofree && buffer )
        MP4Free( buffer );

    mean     = "";
    name     = "";
    type     = BT_UNDEFINED;
    buffer   = NULL;
    size     = 0;
    autofree = false;
}

///////////////////////////////////////////////////////////////////////////////

bool
FreeformBox::add( MP4FileHandle hFile, const Item& item ) {
    MP4File &file = *((MP4File *) hFile);

    int freeformAtomIndex;
    for (freeformAtomIndex = 0; freeformAtomIndex < CHAR_MAX; freeformAtomIndex++) {
        // Freeform
        char freeformPath[256];
        snprintf(freeformPath, sizeof(freeformPath), "moov.udta.meta.ilst.----[%u]", freeformAtomIndex);
        MP4Atom *freeformAtom = file.FindAtom(freeformPath);
        if (!freeformAtom) {
            break;
        }

        // Name
        MP4Atom *freeformNameAtom = freeformAtom->FindChildAtom("name");
        if (!freeformNameAtom) {
            continue;
        }
        MP4BytesProperty *nameBytesProperty = NULL;
        freeformNameAtom->FindProperty("name.value", (MP4Property **) &nameBytesProperty);
        if (!nameBytesProperty) {
            continue;
        }
        string name = item.name;
        uint8_t* nameValue;
        uint32_t nameSize;
        nameBytesProperty->GetValue(&nameValue, &nameSize);
        if (!nameValue || strcmp(name.c_str(), (const char *) nameValue) != 0) {
            continue;
        }

        // Mean
        MP4Atom *freeformMeanAtom = freeformAtom->FindChildAtom("mean");
        if (!freeformMeanAtom) {
            continue;
        }
        MP4BytesProperty *meanBytesProperty = NULL;
        freeformMeanAtom->FindProperty("mean.value", (MP4Property **) &meanBytesProperty);
        if (!meanBytesProperty) {
            continue;
        }
        string mean = item.mean;
        uint8_t* meanValue;
        uint32_t meanSize;
        meanBytesProperty->GetValue(&meanValue, &meanSize);
        if (!meanValue || strcmp(mean.c_str(), (const char *) meanValue) != 0) {
            continue;
        }

        // Data
        MP4Atom *freeformDataAtom = freeformAtom->FindChildAtom("data");
        if (!freeformDataAtom) {
            freeformDataAtom = file.AddDescendantAtoms(freeformAtom, "data");
            if (!freeformDataAtom) {
                return false;
            }
        }

        MP4BytesProperty *dataBytesProperty = NULL;
        freeformDataAtom->FindProperty("data.value", (MP4Property **) &dataBytesProperty);
        dataBytesProperty->SetValue(item.buffer, item.size);
        return true;
    }

    return set( hFile, item, freeformAtomIndex );
}

///////////////////////////////////////////////////////////////////////////////

bool
FreeformBox::get( MP4FileHandle hFile, Item& item, uint32_t index )
{
    item.reset();
    MP4File& file = *((MP4File*)hFile);

    char freeformPath[256];
    snprintf(freeformPath, sizeof(freeformPath), "moov.udta.meta.ilst.----");
    MP4Atom* freeformAtom = file.FindAtom(freeformPath);
    if (!freeformAtom) {
        return true;
    }

    if (!(index < freeformAtom->GetNumberOfChildAtoms())) {
        return true;
    }

    MP4DataAtom* data = static_cast<MP4DataAtom*>(freeformAtom->GetChildAtom(index));
    if(!data) {
        return true;
    }

    MP4BytesProperty* metadata = NULL;
    if (!data->FindProperty( "data.metadata", (MP4Property**)&metadata)) {
        return true;
    }

    metadata->GetValue(&item.buffer, &item.size);
    item.autofree = true;
    item.type = data->typeCode.GetValue();

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
FreeformBox::list( MP4FileHandle hFile, ItemList& out )
{
    out.clear();
    MP4File& file = *((MP4File*)hFile);
    MP4ItmfItemList* itemList = genericGetItemsByCode( file, "----" ); // alloc

    if( itemList->size ) {
        MP4ItmfDataList& dataList = itemList->elements[0].dataList;
        out.resize( dataList.size );
        for( uint32_t i = 0; i < dataList.size; i++ ) {
            get(hFile, out[i], i);
        }
    }

    genericItemListFree( itemList ); // free
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
FreeformBox::remove( MP4FileHandle hFile, uint32_t index )
{
    MP4File& file = *((MP4File*)hFile);

    MP4Atom* freeform = file.FindAtom( "moov.udta.meta.ilst.----" );
    if( !freeform )
        return true;

    // wildcard mode: delete all freeform nodes
    if( index == numeric_limits<uint32_t>::max() ) {
        freeform->GetParentAtom()->DeleteChildAtom( freeform );
        delete freeform;
        return false;
    }

    if( !(index < freeform->GetNumberOfChildAtoms()) )
        return true;

    MP4Atom* data = freeform->GetChildAtom( index );
    if( !data )
        return true;

    // delete single object
    freeform->DeleteChildAtom( data );
    delete data;

    // delete empty freeform container
    if( freeform->GetNumberOfChildAtoms() == 0 ) {
        freeform->GetParentAtom()->DeleteChildAtom( freeform );
        delete freeform;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
FreeformBox::set( MP4FileHandle hFile, const Item& item, uint32_t index ) {
    // mean, name, data
    MP4File &file = *((MP4File *) hFile);

    // Freeform
    char freeformPath[256];
    snprintf(freeformPath, sizeof(freeformPath), "moov.udta.meta.ilst.----[%u]", index);
    MP4Atom *freeformAtom = file.FindAtom(freeformPath);
    if (!freeformAtom) {
        snprintf(freeformPath, sizeof(freeformPath), "udta.meta.ilst.----[%u]", index);
        freeformAtom = file.AddDescendantAtoms("moov", freeformPath);
        if (!freeformAtom) {
            return false;
        }
    }

    // Data
    MP4DataAtom *freeformDataAtom = static_cast<MP4DataAtom *>(freeformAtom->FindChildAtom("data"));
    if (!freeformDataAtom) {
        freeformDataAtom = static_cast<MP4DataAtom *>(file.AddDescendantAtoms(freeformAtom, "data"));
        if (!freeformDataAtom) {
            return false;
        }
    }

    MP4BytesProperty *dataBytesProperty = NULL;
    freeformDataAtom->typeCode.SetValue(BT_UTF8);
    freeformDataAtom->FindProperty("data.metadata", (MP4Property **) &dataBytesProperty);
    if (!dataBytesProperty) {
        return false;
    }
    dataBytesProperty->SetValue(item.buffer, item.size);

    MP4Atom *freeformNameAtom = freeformAtom->FindChildAtom("name");
    if (!freeformNameAtom) {
        freeformNameAtom = file.AddDescendantAtoms(freeformAtom, "name");
        if (!freeformNameAtom) {
            return false;
        }
    }

    // Name
    MP4BytesProperty *nameStringProperty = NULL;
    freeformNameAtom->FindProperty("name.value", (MP4Property **) &nameStringProperty);
    if (!nameStringProperty) {
        return false;
    }
    string name = item.name;
    const uint8_t *nameBytes = reinterpret_cast<const uint8_t *>(name.c_str());
    nameStringProperty->SetValue(nameBytes, name.length());
    MP4Atom *freeformMeanAtom = freeformAtom->FindChildAtom("mean");
    if (!freeformMeanAtom) {
        freeformMeanAtom = file.AddDescendantAtoms(freeformAtom, "mean");
        if (!freeformMeanAtom) {
            return false;
        }
    }

    // Mean
    MP4BytesProperty *meanStringProperty = NULL;
    freeformMeanAtom->FindProperty("mean.value", (MP4Property **) &meanStringProperty);
    if (!meanStringProperty) {
        return false;
    }
    string mean = item.mean;
    if (mean.length() == 0) {
        mean = "com.apple.iTunes";
    }
    const uint8_t *meanBytes = reinterpret_cast<const uint8_t *>(mean.c_str());
    meanStringProperty->SetValue(meanBytes, mean.length());

    return true;
}

///////////////////////////////////////////////////////////////////////////////

}}} // namespace mp4v2::impl::itmf
