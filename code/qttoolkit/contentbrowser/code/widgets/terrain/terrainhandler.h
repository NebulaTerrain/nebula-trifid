#pragma once
//------------------------------------------------------------------------------
/**
	@class Widgets::MaterialHandler
	
	Handles the UI which modifies a single surface material.
	
	(C) 2015 Individual contributors, see AUTHORS file
*/
//------------------------------------------------------------------------------
#include "widgets/basehandler.h"
#include "ui_terrainwidget.h"
#include "widgets/materials/mutablesurface.h"
#include "widgets/materials/mutablesurfaceinstance.h"
#include "materials/managedsurface.h"
#include "resources/managedtexture.h"
#include <QObject>
#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include "models/nodes/statenodeinstance.h"
#include "n3util/n3modeldata.h"
#include "ui_saveresourcedialog.h"
#include "terrainaddon/terrainaddon.h"

namespace Widgets
{
class TerrainHandler : public BaseHandler
{
	Q_OBJECT
		__DeclareClass(TerrainHandler);
public:
	/// constructor
	TerrainHandler();
	/// destructor
	virtual ~TerrainHandler();

    /// setup
    void Setup(const QString& resource);
    /// discard
    bool Discard();

    /// sets the ui on which this handler shall perform its actions
    void SetUI(Ui::TerrainWidget* ui);
    /// gets pointer to ui
    Ui::TerrainWidget* GetUI() const;

    /// called whenever a texture has been changed
    void TextureChanged(uint i);
    /// called whenever a float parameter has been changed
    void FloatVariableChanged(uint i);
    /// called whenever a float2 parameter has been changed
    void Float2VariableChanged(uint i);
    /// called whenever a float4 parameter has been changed
    void Float4VariableChanged(uint i);
    /// called whenever a bool parameter has been changed
    void BoolVariableChanged(uint i);
    /// called whenever an int variable has been changed
    void IntVariableChanged(uint i);

    /// called whenever a float field limit has changed
    void FloatLimitChanged(uint i);
    /// called whenever an int field limit has changed
    void IntLimitChanged(uint i);

public slots:
	/// called when we should make a new material
	void NewSurface();

private slots:

	void NewTerrain();

	void GenerateTerrain();

	void FlattenTerrain();

	void ApplyHeightMultiplier();

	/// called whenever height scale slider is changed
	void UpdateHeightMultiplier(double multiplier);

	void BlurTerrain();

	void UpdateBrushStrength(double strength);

	void UpdateBrushSize(int size);

	void UpdateBrushMaxheight(double);

    /// called whenever a material is selected
    void MaterialSelected(const QString& material);
    /// called whenever the material info button is clicked
    void MaterialInfo();

    /// called whenever a line edit is finished
    void TextureTextChanged();
    /// called whenever an int slider has changed its value
    void VariableIntSliderChanged();
    /// called whenever an int slider has stopped
    void VariableIntSliderDone();
    /// called whenever a float slider has changed its value
    void VariableFloatSliderChanged();
    /// called whenever a float slider has stopped
    void VariableFloatSliderDone();
    /// called whenever an int field has changed its value
    void VariableIntFieldChanged();
    /// called whenever a float field has changed its value
    void VariableFloatFieldChanged();
    /// called whenever a float2 field has changed its value
    void VariableFloat2FieldChanged();
    /// called whenever a float4 field has changed its value
    void VariableFloat4FieldChanged();
    /// called whenever a bool check box has changed its value
    void VariableCheckBoxChanged();

    /// called whenever a float limit box changed its value
    void VariableFloatLimitsChanged();
    /// called whenever an int limit box changed its value
    void VariableIntLimitsChanged();

    /// called whenever the browse button is pressed
    void Browse();

	/// called whenever the save button is clicked
	void Save();
	/// save material as another file
	void SaveAs();

	/// called whenever we create a new dialog
	void OnNewCategory();

protected:
    /// sets up texture selection button and line edit based on resource
    void SetupTextureSlotHelper(QLineEdit* textureField, QPushButton* textureButton, Util::String& resource, const Util::String& defaultResource);
    /// setup material variables and textures
    void MakeMaterialUI(QComboBox* materialBox, QPushButton* materialHelp);

    /// get material variables which are textures
    Util::Array<Materials::Material::MaterialParameter> GetTextures(const Ptr<Materials::Material>& mat);
    /// get material variables which aren't textures
    Util::Array<Materials::Material::MaterialParameter> GetVariables(const Ptr<Materials::Material>& mat);

    /// clears layout recursively
    void ClearFrame(QLayout* layout);

private:
	/// setup save dialog
	void SetupSaveDialog();
	/// open save dialog
	int OpenSaveDialog();	
	/// helper function to reset the UI
	void ResetUI();
	/// handle modifications
	void OnModified();

	/// update thumbnail
	void UpdateThumbnail();

	Ptr<Terrain::TerrainAddon> terrainAddon;

    QVBoxLayout* mainLayout;
    QComboBox* materialBox;
    QPushButton* materialHelp;

    // default values
    QMap<uint, Util::Variant> defaultValueMap;
    QMap<uint, QString> defaultTextureMap;

    // texture
    QMap<QLineEdit*, uint> textureTextMap;
    QMap<QPushButton*, uint> textureImgMap;
    QMap<QLabel*, uint> textureLabelMap;

    // label of variable (generic)
    QMap<QLabel*, uint> variableLabelMap;

    // integer
    QMap<QSpinBox*, uint> variableIntValueMap;
    QMap<QSpinBox*, uint> lowerLimitIntMap;
    QMap<QSpinBox*, uint> upperLimitIntMap;

    // float
    QMap<QDoubleSpinBox*, uint> variableFloatValueMap;
    QMap<QDoubleSpinBox*, uint> lowerLimitFloatMap;
    QMap<QDoubleSpinBox*, uint> upperLimitFloatMap;

    // slider for float or integer
    QMap<QSlider*, uint> variableSliderMap;

    // float2, float4
    QMap<QDoubleSpinBox*, uint> variableVectorFieldMap;
    QMap<uint, QList<QDoubleSpinBox*> > variableVectorMap;
    QMap<QPushButton*, uint> variableVectorColorEditMap;

    // bool
    QMap<QCheckBox*, uint> variableBoolMap;
	Ptr<Materials::ManagedSurface> managedSurface;
    Ptr<Materials::MutableSurface> surface;
	Ptr<Materials::MutableSurfaceInstance> surfaceInstance;
    Util::Dictionary<IndexT, Ptr<Resources::ManagedTexture>> textureResources;
    Util::Dictionary<IndexT, Util::StringAtom> textureVariables;
    Util::Dictionary<IndexT, Util::StringAtom> scalarVariables;
    ToolkitUtil::State state;

	// used to hold current color when color picking
	Math::float4 currentColor;
	QColorDialog* colorDialog;

	QDialog saveDialog;
	Ui::SaveResourceDialog saveDialogUi;
    Ui::TerrainWidget* ui;

	bool hasChanges;
	Util::String category;
	Util::String file;
};

//------------------------------------------------------------------------------
/**
	Ehh, do this only once...
*/
inline void
TerrainHandler::SetUI(Ui::TerrainWidget* ui)
{
    this->ui = ui;

	connect(this->ui->heightMapSize_horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(ui->heightMapSize_spinBox->setValue(int))); //visual
	connect(this->ui->heightMapSize_spinBox, SIGNAL(valueChanged(int)), this, SLOT(ui->heightMapSize_horizontalSlider->setValue(int))); //visual
	connect(this->ui->new_pushButton, SIGNAL(clicked()), this, SLOT(NewTerrain()));
	connect(this->ui->generate_pushButton, SIGNAL(clicked()), this, SLOT(GenerateTerrain()));

	//connect(this->ui->heightScale_horizontalSlider, SIGNAL(valueChanged(double)), this, SLOT(ui->heightScale_doubleSpinBox->setValue(double))); //visual
	//connect(this->ui->heightScale_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(ui->heightScale_horizontalSlider->setValue(double))); //visual
	connect(this->ui->heightScale_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(UpdateHeightMultiplier(double)));
	connect(this->ui->flattenHeightMap_pushButton, SIGNAL(clicked()), this, SLOT(FlattenTerrain()));
	connect(this->ui->applyScale_pushButton, SIGNAL(clicked()), this, SLOT(ApplyHeightMultiplier()));

	connect(this->ui->fullBlurStrength_horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(ui->fullBlurStrength_spinBox->setValue(int))); //visual
	connect(this->ui->fullBlurStrength_spinBox, SIGNAL(valueChanged(int)), this, SLOT(ui->fullBlurStrength_horizontalSlider->setValue(int))); //visual
	connect(this->ui->fullBlurHeightMap_pushButton, SIGNAL(clicked()), this, SLOT(BlurTerrain()));

	//connect(this->ui->strength_horizontalSlider, SIGNAL(valueChanged(double)), this, SLOT(ui->strength_doubleSpinBox->setValue(double))); //visual
	//connect(this->ui->strength_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(ui->strength_horizontalSlider->setValue(double))); //visual
	connect(this->ui->strength_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(UpdateBrushStrength(double)));

	connect(this->ui->size_horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(ui->size_spinBox->setValue(int))); //visual
	connect(this->ui->size_spinBox, SIGNAL(valueChanged(int)), this, SLOT(ui->size_horizontalSlider->setValue(int))); //visual
	connect(this->ui->size_spinBox, SIGNAL(valueChanged(int)), this, SLOT(UpdateBrushSize(int)));

	//connect(this->ui->maxHeight_horizontalSlider, SIGNAL(valueChanged(double)), this, SLOT(ui->maxHeight_doubleSpinBox->setValue(double))); //visual
	//connect(this->ui->maxHeight_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(ui->maxHeight_horizontalSlider->setValue(double))); //visual
	connect(this->ui->maxHeight_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(UpdateBrushMaxheight(double)));

	// setup terrain
	this->terrainAddon = Terrain::TerrainAddon::Create();
	
    //this->ui->templateBox->setEnabled(false);
    //this->ui->saveButton->setEnabled(false);
    //this->ui->saveAsButton->setEnabled(false);

	// hmm, this shouldn't really be done each time we open a surface...
	//connect(this->ui->saveButton, SIGNAL(clicked()), this, SLOT(Save()));
	//connect(this->ui->saveAsButton, SIGNAL(clicked()), this, SLOT(SaveAs()));
	//connect(this->ui->templateBox, SIGNAL(activated(const QString&)), this, SLOT(MaterialSelected(const QString&)));
	//connect(this->ui->materialHelp, SIGNAL(clicked()), this, SLOT(MaterialInfo()));
}

//------------------------------------------------------------------------------
/**
*/
inline Ui::TerrainWidget*
TerrainHandler::GetUI() const
{
    return this->ui;
}

} // namespace Widgets