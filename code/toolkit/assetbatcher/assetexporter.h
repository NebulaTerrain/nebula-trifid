#pragma once
//------------------------------------------------------------------------------
/**
	@class ToolkitUtil::AssetExporter
	
	The asset exporter takes a single directory and exports any models, textures and gfx-sources.

    This isn't based on an exporter class, because it has no need for incremental batching.
	
	(C) 2015 Individual contributors, see AUTHORS file
*/
//------------------------------------------------------------------------------
#include "toolkitutil/base/exporterbase.h"
#include "toolkitutil/fbx/nfbxexporter.h"
#include "texutil/textureconverter.h"
#include "modelutil/modelbuilder.h"
#include "modelutil/modeldatabase.h"
namespace ToolkitUtil
{
class AssetExporter : public Base::ExporterBase
{
	__DeclareClass(AssetExporter);
public:

    enum ExportModes
    {
        FBX = 1 << 0,                   // checking this will cause FBXes to get exported
        Models = 1 << 1,                // checking this will cause models to get exported
        Textures = 1 << 2,              // checking this will cause textures to get exported
        All = FBX + Models + Textures,  // shortcut for exporting everything

        ForceFBX = 1 << 3,              // will force the FBX batcher to update meshes and characters despite time stamps
        ForceModels = 1 << 4,           // will force the model builder to create models despite time stamps
        ForceTextures = 1 << 5,         // will force the texture converter to convert textures despite time stamps
        ForceAll = ForceFBX + ForceModels + ForceTextures
    };

	/// constructor
	AssetExporter();
	/// destructor
	virtual ~AssetExporter();

    /// opens the exporter
    void Open();
    /// closes the exporter
    void Close();
    /// returns true if exporter is open
    bool IsOpen() const;

	/// explicitly exports the system directories (toolkit:system and toolkit:lighting)
	void ExportSystem();
    /// exports a single directory
    void ExportDir(const Util::String& category);
    /// exports all files
    void ExportAll();

    /// exports list of files, used for parallel jobs
    void ExportList(const Util::Array<Util::String>& files);

private:
    Ptr<ToolkitUtil::NFbxExporter> fbxExporter;
    ToolkitUtil::TextureConverter textureExporter;
    Ptr<ToolkitUtil::ModelBuilder> modelBuilder;
    Logger logger;
    ExportModes mode;
};
} // namespace ToolkitUtil