/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformableNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
///  vtkMRMLTransformableNode - MRML node for representing a node with a tranform
/// 
/// A supercalss for other nodes that can have a transform to parent node 
/// like volume, model and transformation nodes

#ifndef __vtkMRMLTransformableNode_h
#define __vtkMRMLTransformableNode_h

// MRML includes
#include "vtkMRMLNode.h"
class vtkMRMLTransformNode;

// VTK includes
class vtkAbstractTransform;
class vtkMatrix4x4;

class VTK_MRML_EXPORT vtkMRMLTransformableNode : public vtkMRMLNode
{
public:
  vtkTypeMacro(vtkMRMLTransformableNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  /// 
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() = 0;

  /// 
  /// Updates this node if it depends on other nodes 
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  /// 
  /// Observe the reference transform node
  virtual void UpdateScene(vtkMRMLScene *scene);

  /// 
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  /// 
  /// String ID of the transform MRML node
  vtkGetStringMacro(TransformNodeID);
  void SetAndObserveTransformNodeID(const char *transformNodeID);

  /// 
  /// Associated transform MRML node
  vtkMRMLTransformNode* GetParentTransformNode();

  /// 
  /// alternative method to propagate events generated in Transform nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                  unsigned long /*event*/, 
                                  void * /*callData*/ );

  /// TransformModifiedEvent is send when the parent transform is modidied
  enum
    {
      TransformModifiedEvent = 15000
    };

  /// Returns true if the transformable node can apply non linear transforms
  /// \sa ApplyTransformMatrix, ApplyTransform
  virtual bool CanApplyNonLinearTransforms()const;

  /// Concatenate a matrix to the current transform matrix.
  /// \sa SetAndObserveTransformNodeID, ApplyTransform,
  /// CanApplyNonLinearTransforms
  virtual void ApplyTransformMatrix(vtkMatrix4x4* transformMatrix);

  /// Concatenate a transform to the current transform matrix.
  /// \sa SetAndObserveTransformNodeID, ApplyMatrix,
  /// CanApplyNonLinearTransforms
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  /// Apply the observed transform to the input point.
  /// \sa TransformPointFromWorld, SetAndObserveTransformNodeID
  virtual void TransformPointToWorld(const double in[4], double out[4]);

  /// Apply the invert of the observed transform to the input point.
  /// \sa TransformPointToWorld, SetAndObserveTransformNodeID
  virtual void TransformPointFromWorld(const double in[4], double out[4]);

protected:
  vtkMRMLTransformableNode();
  ~vtkMRMLTransformableNode();
  vtkMRMLTransformableNode(const vtkMRMLTransformableNode&);
  void operator=(const vtkMRMLTransformableNode&);

  void SetTransformNodeID(const char* id);
  char *TransformNodeID;

  vtkMRMLTransformNode* TransformNode;

};

#endif
