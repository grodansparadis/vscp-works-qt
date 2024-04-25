// cdlgmdfbootloader.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2024 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef CDLGMDFBOOTLOADER_H
#define CDLGMDFBOOTLOADER_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>

// Id for fields to identify them in the UI.
// Use to set focus on edit and to write changed values 
// in correct position among other things.
typedef enum mdf_bootloader_index {
  index_bootloader_none = 0,
  index_bootloader_blocksize,
  index_bootloader_blockcount,
  index_bootloader_algorithm,
} mdf_bootloader_index;

// Tree prefixes (Text before value in tree table)
#define PREFIX_MDF_BOOTLOADER_BLOCKSIZE tr("Block size : ")
#define PREFIX_MDF_BOOTLOADER_NUMBER_OF_BLOCKS tr("Number of blocks: ")
#define PREFIX_MDF_BOOTLOADER_ALGORITHM tr("Algorithm: ")

namespace Ui {
class CDlgMdfBootLoader;
}

class CDlgMdfBootLoader : public QDialog {
  Q_OBJECT

public:
  explicit CDlgMdfBootLoader(QWidget* parent = nullptr);
  ~CDlgMdfBootLoader();

  /*!
      Set inital focus to description
  */
  void setInitialFocus(void);

  /*!
    Init dialog data
    @param pmdfobject Pointer to MDF object
    @param index The index for the field that will get focus
      
  */
  void initDialogData(const CMDF_Object* pmdfobj, mdf_bootloader_index index = index_bootloader_blocksize);


  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  /*!
      Name getter/setters
  */
  void setBlockSize(uint32_t size);
  uint32_t getBlockSize(void);

  /*!
      GUID getter/setters
  */
  void setNumberOfBlocks(uint32_t nob);
  uint32_t getNumberOfBlocks(void);

  /*!
      GUID getter/setters
  */
  void setAlgorithm(uint16_t nAlgorithm);
  uint16_t getAlgorithm(void);


public slots:
  void accept(void);


private:
  Ui::CDlgMdfBootLoader* ui;
  CMDF_BootLoaderInfo* m_pbootinfo;
};

#endif // CDLGMDFBOOTLOADER_H
