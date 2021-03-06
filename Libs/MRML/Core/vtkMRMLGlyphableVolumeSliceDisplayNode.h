/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLGlyphableVolumeSliceDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

  =========================================================================auto=*/
///  vtkMRMLGlyphableVolumeSliceDisplayNode - MRML node to represent display properties for tractography.
/// 
/// vtkMRMLGlyphableVolumeSliceDisplayNode nodes store display properties of trajectories 
/// from tractography in diffusion MRI data, including color type (by bundle, by fiber, 
/// or by scalar invariants), display on/off for tensor glyphs and display of 
/// trajectory as a line or tube.
//

#ifndef __vtkMRMLGlyphableVolumeSliceDisplayNode_h
#define __vtkMRMLGlyphableVolumeSliceDisplayNode_h

#include <string>


#include "vtkMRML.h"
#include "vtkMRMLModelDisplayNode.h"

class vtkTransform;
class vtkTransformPolyDataFilter;
class vtkMatrix4x4;
class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLGlyphableVolumeSliceDisplayNode : public vtkMRMLModelDisplayNode
{
 public:
  static vtkMRMLGlyphableVolumeSliceDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLGlyphableVolumeSliceDisplayNode,vtkMRMLModelDisplayNode );
  void PrintSelf ( ostream& os, vtkIndent indent );
  
  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance (  );

  /// 
  /// Read node attributes from XML (MRML) file
  virtual void ReadXMLAttributes ( const char** atts );

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML ( ostream& of, int indent );


  /// 
  /// Copy the node's attributes to this object
  virtual void Copy ( vtkMRMLNode *node );
  
  /// 
  /// Get node XML tag name (like Volume, UnstructuredGrid)
  virtual const char* GetNodeTagName ( ) {return "GlyphableVolumeSliceDisplayNode";};

  /// 
  /// Updates this node if it depends on other nodes 
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  virtual void UpdateReferenceID(const char *oldID, const char *newID)
    { Superclass::UpdateReferenceID(oldID, newID); };

  /// 
  /// Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);


  /// 
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );


  /// 
  /// Sets polydata for glyph input
  virtual void SetPolyData(vtkPolyData *glyphPolyData);

  /// 
  /// Gets resulting glyph PolyData 
  virtual vtkPolyData* GetPolyData();
  
  /// 
  /// Gets resulting glyph PolyData transfomed to slice XY
  virtual vtkPolyData* GetPolyDataTransformedToSlice();
   
  /// 
  /// Update the pipeline based on this node attributes
  virtual void UpdatePolyDataPipeline();

  /// 
  /// Set ImageData for a volume slice
  virtual void SetSliceImage(vtkImageData *image);
 
  /// 
  /// Set slice to RAS transformation
  virtual void SetSlicePositionMatrix(vtkMatrix4x4 *matrix);

  /// 
  /// Set slice to IJK transformation
  virtual void SetSliceGlyphRotationMatrix(vtkMatrix4x4 *matrix);

  //--------------------------------------------------------------------------
  /// Display Information: Geometry to display (not mutually exclusive)
  //--------------------------------------------------------------------------


  //--------------------------------------------------------------------------
  /// Display Information: Color Mode
  /// 0) solid color by group 1) color by scalar invariant 
  /// 2) color by avg scalar invariant 3) color by other
  //--------------------------------------------------------------------------

  enum
  {
    colorModeSolid = 0,
    colorModeScalar = 1,
    colorModeFunctionOfScalar = 2,
    colorModeUseCellScalars = 3
  };

  //--------------------------------------------------------------------------
  /// Display Information: ColorMode for ALL nodes
  //--------------------------------------------------------------------------

 /// Description:
  /// Color mode for glyphs. The color modes are mutually exclusive.
  vtkGetMacro ( ColorMode, int );
  vtkSetMacro ( ColorMode, int );
 
  /// 
  /// Color by solid color (for example the whole fiber bundle red. blue, etc.)
  void SetColorModeToSolid ( ) {
    this->SetColorMode ( this->colorModeSolid );
  };

  /// 
  /// Color according to the tensors using various scalar invariants.
  void SetColorModeToScalar ( ) {
    this->SetColorMode ( this->colorModeScalar );
  };

  /// 
  /// Color according to the tensors using a function of scalar invariants along the tract.
  /// This enables coloring by average FA, for example.
  void SetColorModeToFunctionOfScalar ( ) {
    this->SetColorMode ( this->colorModeFunctionOfScalar );
  };

  /// 
  /// Use to color by the active cell scalars.  This is intended to support
  /// external processing of fibers, for example to label each with the distance
  /// of that fiber from an fMRI activation.  Then by making that information
  /// the active cell scalar field, this will allow coloring by that information.
  /// TO DO: make sure this information can be saved with the tract, save name of
  /// active scalar field if needed.
  void SetColorModeToUseCellScalars ( ) {
    this->SetColorMode ( this->colorModeUseCellScalars );
  };



  //--------------------------------------------------------------------------
  /// Display Information: ColorMode for glyphs
  //--------------------------------------------------------------------------
  

  //--------------------------------------------------------------------------
  /// MRML nodes that are observed
  //--------------------------------------------------------------------------
 protected:
  vtkMRMLGlyphableVolumeSliceDisplayNode ( );
  ~vtkMRMLGlyphableVolumeSliceDisplayNode ( );
  vtkMRMLGlyphableVolumeSliceDisplayNode ( const vtkMRMLGlyphableVolumeSliceDisplayNode& );
  void operator= ( const vtkMRMLGlyphableVolumeSliceDisplayNode& );

    vtkImageData             *SliceImage;
    vtkTransform             *SliceToXYTransform;
    vtkTransformPolyDataFilter *SliceToXYTransformer;
    vtkMatrix4x4             *SliceToXYMatrix;


    /// Enumerated
    int ColorMode;


};

#endif
