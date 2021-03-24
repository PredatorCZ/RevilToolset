/*  TEXConvert
    Copyright(C) 2020 Lukas Cone

    This program is free software : you can redistribute it and / or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see <https://www.gnu.org/licenses/>.
*/

#include "datas/DirectoryScanner.hpp"
#include "datas/MultiThread.hpp"
#include "datas/SettingsManager.hpp"
#include "datas/binreader.hpp"
#include "datas/binwritter.hpp"
#include "datas/fileinfo.hpp"
#include "datas/reflectorRegistry.hpp"
#include "formats/DDS.hpp"
#include "project.h"
#include "pugixml.hpp"

static struct TEXConvert : SettingsManager {
  DECLARE_REFLECTOR;

  bool Generate_Log = false;
  bool Convert_DDS_to_legacy = true;
  bool Force_unconvetional_legacy_formats = true;
  bool Folder_scan_TEX_only = true;
  bool Extract_largest_mipmap = false;
} settings;

REFLECTOR_START_WNAMES(TEXConvert, Convert_DDS_to_legacy,
                       Force_unconvetional_legacy_formats,
                       Extract_largest_mipmap, Generate_Log);

static const char help[] = "\nConverts TEX textures.\n\
Settings (.config file):\n\
  Convert_DDS_to_legacy: \n\
        Tries to convert TEX into legacy (DX9) DDS format.\n\
  Force_unconvetional_legacy_formats:\n\
        Will try to convert some matching formats from DX10 to DX9,\n\
        for example: RG88 to AL88.\n\
  Extract_largest_mipmap:\n\
        Will try to extract only highest mipmap.\n\
        Texture musn't be converted back afterwards, unless you regenerate mipmaps!\n\
        This setting does not apply, if texture have arrays or is a cubemap!\n\
  Generate_Log:\n\
        Will generate text log of console output next to application location.\n\t ";

static const char pressKeyCont[] = "\nPress ENTER to close.";

struct RETEXMip {
  char *offset;
  int pad;
  uint unk;
  uint size;
};

struct RETEX {
  static const int ID = CompileFourCC("TEX\0");
  int id, version;
  ushort width, height,
      depth; // vector field
  uchar numMips;
  uchar numArrays;
  DXGI_FORMAT format;
  int unk01; // -1
  int unk;   // 4 = cubemap
  int flags;

  const RETEXMip *Mips() const {
    return reinterpret_cast<const RETEXMip *>(this + 1);
  }

  void Fixup() {
    char *root = reinterpret_cast<char *>(this);
    RETEXMip *mips = reinterpret_cast<RETEXMip *>(this + 1);
    int numTotalMips = numMips * numArrays;

    for (int t = 0; t < numTotalMips; t++) {
      mips[t].offset = root + reinterpret_cast<uint>(mips[t].offset);
    }
  }
};

void FilehandleITFC(const TSTRING &fle) {
  printline("Loading file: ", << fle);
  BinReader rd(fle);

  if (!rd.IsValid()) {
    printerror("Cannot open file.");
    return;
  }

  int ID;
  rd.Read(ID);
  rd.Seek(0);

  if (ID == RETEX::ID) {
    const size_t fleSize = rd.GetSize();
    std::string buffer;
    rd.ReadContainer(buffer, fleSize);
    RETEX *tex = reinterpret_cast<RETEX *>(&buffer[0]);
    tex->Fixup();

    TFileInfo fleInfo0 = fle;
    TFileInfo fleInfo1 = fleInfo0.GetFileName();
    TSTRING outFile = fleInfo0.GetPath() + fleInfo1.GetFileName() + _T(".dds");
    std::ofstream ofs(outFile, std::ios::out | std::ios::binary);

    DDS ddtex = {};
    ddtex = DDSFormat_DX10;
    ddtex.dxgiFormat = tex->format;
    ddtex.width = tex->width;
    ddtex.height = tex->height;

    if (tex->depth > 1) {
      ddtex.depth = tex->depth;
      ddtex.flags += DDS::Flags_Depth;
      ddtex.caps01 += DDS_HeaderEnd::Caps01Flags_Volume;
    } else if (tex->unk != 4) {
      ddtex.arraySize = tex->numArrays;
    } else {
      ddtex.caps01 = decltype(ddtex.caps01)(DDS::Caps01Flags_CubeMap,
                                            DDS::Caps01Flags_CubeMap_NegativeX,
                                            DDS::Caps01Flags_CubeMap_NegativeY,
                                            DDS::Caps01Flags_CubeMap_NegativeZ,
                                            DDS::Caps01Flags_CubeMap_PositiveX,
                                            DDS::Caps01Flags_CubeMap_PositiveY,
                                            DDS::Caps01Flags_CubeMap_PositiveZ);
    }

    ddtex.NumMipmaps(settings.Extract_largest_mipmap ? 1 : tex->numMips);

    const int sizetoWrite =
        !settings.Convert_DDS_to_legacy || ddtex.arraySize > 1 ||
                ddtex.ToLegacy(settings.Force_unconvetional_legacy_formats)
            ? ddtex.DDS_SIZE
            : ddtex.LEGACY_SIZE;

    if (settings.Convert_DDS_to_legacy && sizetoWrite == ddtex.DDS_SIZE) {
      printwarning("Couldn't convert DX10 dds to legacy.")
    }

    ofs.write(reinterpret_cast<const char *>(&ddtex), sizetoWrite);

    const RETEXMip *mips = tex->Mips();
    const int mipPerArray = tex->numMips;

    for (int a = 0; a < tex->numArrays; a++) {
      for (int m = 0; m < ddtex.mipMapCount; m++) {
        const RETEXMip &cMip = mips[m + mipPerArray * a];
        ofs.write(cMip.offset, cMip.size * tex->depth);
      }
    }

    ofs.close();
  }
}

struct TexQueueTraits {
  int queue;
  int queueEnd;
  TCHAR **files;
  typedef void return_type;

  return_type RetreiveItem() {
    TSTRING filepath = files[queue];

    if (filepath.find('.') == filepath.npos)
      return;

    FilehandleITFC(filepath);
  }

  operator bool() { return queue < queueEnd; }
  void operator++(int) { queue++; }
  int NumQueues() const { return queueEnd - 1; }
};

struct TexFolderQueueTraits {
  int queue = 0;
  int queueEnd;
  DirectoryScanner ds;
  typedef void return_type;

  return_type RetreiveItem() {
    const TSTRING &filepath = ds.Files()[queue];
    FilehandleITFC(filepath);
  }

  operator bool() { return queue < queueEnd; }
  void operator++(int) { queue++; }
  int NumQueues() const { return queueEnd; }
};

int _tmain(int argc, TCHAR *argv[]) {
  setlocale(LC_ALL, "");
  printer.AddPrinterFunction(wprintf);

  printline(TEXConvert_DESC ", " TEXConvert_COPYRIGHT
                            "\nSimply drag'n'drop files into application or "
                            "use as " TEXConvert_PRODUCT_NAME
                            " file1 file2 ...\n");

  TFileInfo configInfo(*argv);
  const TSTRING configName =
      configInfo.GetPath() + configInfo.GetFileName() + _T(".config");

  settings.FromXML(configName);

  pugi::xml_document doc = {};
  pugi::xml_node mainNode(settings.ToXML(doc));
  mainNode.prepend_child(pugi::xml_node_type::node_comment).set_value(help);

  doc.save_file(configName.c_str(), "\t",
                pugi::format_write_bom | pugi::format_indent);

  if (argc < 2) {
    printerror("Insufficient argument count, expected at least 1.");
    printer << help << pressKeyCont >> 1;
    getchar();
    return 1;
  }

  if (argv[1][1] == '?' || argv[1][1] == 'h') {
    printer << help << pressKeyCont >> 1;
    getchar();
    return 0;
  }

  if (settings.Generate_Log)
    settings.CreateLog(configInfo.GetPath() + configInfo.GetFileName());

  printer.PrintThreadID(true);

  TexQueueTraits texQue;
  texQue.files = argv;
  texQue.queue = 1;
  texQue.queueEnd = argc;

  RunThreadedQueue(texQue);

  std::vector<const TCHAR *> folders;

  for (int a = 1; a < argc; a++) {
    const TCHAR *curItem = argv[a];

    while (*curItem) {
      if (*curItem == '.')
        break;

      curItem++;
    }

    if (!*curItem)
      folders.push_back(argv[a]);
  }

  if (folders.size()) {
    printer.PrintThreadID(false);
    printline("Scanning folders for ",
              << (settings.Folder_scan_TEX_only ? "TEX" : "DDS") << " files.");

    TexFolderQueueTraits flQue;
    flQue.ds.AddFilter(settings.Folder_scan_TEX_only ? _T(".tex.")
                                                     : _T(".dds"));

    size_t lastFileCount = 0;

    for (auto &f : folders) {
      printline("Scanning: ", << f);
      flQue.ds.Scan(f);
      printline("Files found: ", << flQue.ds.Files().size() - lastFileCount);
      lastFileCount = flQue.ds.Files().size();
    }

    printline("Scanning done, total files found: ", << lastFileCount);
    flQue.queueEnd = static_cast<int>(lastFileCount);
    printer.PrintThreadID(true);
    RunThreadedQueue(flQue);
  }

  return 0;
}