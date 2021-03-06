/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerTractographyFiducialSeedingLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkSlicerTractographyFiducialSeedingLogic.h"

// MRML includes
#include <vtkMRMLFiberBundleDisplayNode.h>
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLAnnotationHierarchyNode.h>
#include <vtkMRMLAnnotationControlPointsNode.h>
#include <vtkMRMLAnnotationFiducialNode.h>
#include <vtkMRMLFiberBundleNode.h>
#include <vtkMRMLFiberBundleStorageNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkSeedTracts.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkMaskPoints.h>
#include <vtkImageChangeInformation.h>
#include <vtkSmartPointer.h>
#include <vtkImageCast.h>
#include <vtkDiffusionTensorMathematics.h>
#include <vtkImageThreshold.h>

#include "vtkMRMLTractographyFiducialSeedingNode.h"

// ITKSYS includes

// STD includes
#include <algorithm>

vtkCxxRevisionMacro(vtkSlicerTractographyFiducialSeedingLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerTractographyFiducialSeedingLogic);

//----------------------------------------------------------------------------
vtkSlicerTractographyFiducialSeedingLogic::vtkSlicerTractographyFiducialSeedingLogic()
{
  this->MaskPoints = vtkMaskPoints::New();
  this->TractographyFiducialSeedingNode = NULL;
  this->DiffusionTensorVolumeNode = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerTractographyFiducialSeedingLogic::~vtkSlicerTractographyFiducialSeedingLogic()
{
  this->MaskPoints->Delete();
  this->RemoveMRMLNodesObservers();
  vtkSetAndObserveMRMLNodeMacro(this->TractographyFiducialSeedingNode, NULL);
  vtkSetAndObserveMRMLNodeMacro(this->DiffusionTensorVolumeNode, NULL);

}

//----------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic::RemoveMRMLNodesObservers()
{
  for (unsigned int i=0; i<this->ObservedNodes.size(); i++)
    {
    vtkSetAndObserveMRMLNodeMacro(this->ObservedNodes[i], NULL);
    }
  this->ObservedNodes.clear();
}

//----------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
}


//----------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic::SetAndObserveTractographyFiducialSeedingNode(vtkMRMLTractographyFiducialSeedingNode *node)
{
  vtkMRMLTractographyFiducialSeedingNode *oldNode = this->TractographyFiducialSeedingNode;

  vtkSetAndObserveMRMLNodeMacro(this->TractographyFiducialSeedingNode, node);

  if (node && node != oldNode)
    {
    this->RemoveMRMLNodesObservers();

    this->AddMRMLNodesObservers();

    return;
    }

  this->ProcessMRMLNodesEvents(this->TractographyFiducialSeedingNode, vtkCommand::ModifiedEvent, NULL);
  //this->RemoveMRMLNodesObservers();

  //this->AddMRMLNodesObservers();

  return;
}

//----------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic::AddMRMLNodesObservers()
{
  if (this->TractographyFiducialSeedingNode)
    {

    vtkMRMLDiffusionTensorVolumeNode *dtiNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(
          this->GetMRMLScene()->GetNodeByID(this->TractographyFiducialSeedingNode->GetInputVolumeRef()));
    vtkSetAndObserveMRMLNodeMacro(this->DiffusionTensorVolumeNode, dtiNode);

    vtkMRMLNode *seedinNode = this->GetMRMLScene()->GetNodeByID(this->TractographyFiducialSeedingNode->GetInputFiducialRef());

    vtkMRMLAnnotationHierarchyNode *annotationHierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(seedinNode);
    vtkMRMLTransformableNode *transformableNode = vtkMRMLTransformableNode::SafeDownCast(seedinNode);

    if (annotationHierarchyNode)
      {
      this->ObservedNodes.push_back(NULL);
      vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
      events->InsertNextValue ( vtkMRMLHierarchyNode::ChildNodeAddedEvent );
      events->InsertNextValue ( vtkMRMLHierarchyNode::ChildNodeRemovedEvent );
      events->InsertNextValue ( vtkMRMLNode::HierarchyModifiedEvent );

      vtkSetAndObserveMRMLNodeEventsMacro(this->ObservedNodes[this->ObservedNodes.size()-1], 
                                          annotationHierarchyNode, events);

      vtkCollection *annotationNodes = vtkCollection::New();
      annotationHierarchyNode->GetDirectChildren(annotationNodes);
      int nf = annotationNodes->GetNumberOfItems();
      for (int f=0; f<nf; f++)
        {
        vtkMRMLAnnotationControlPointsNode *annotationNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(
                                                      annotationNodes->GetItemAsObject(f));   
        if (annotationNode)
          {
          this->ObservedNodes.push_back(NULL);
          vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
          events->InsertNextValue ( vtkMRMLTransformableNode::TransformModifiedEvent );
          events->InsertNextValue ( vtkMRMLModelNode::PolyDataModifiedEvent );
          events->InsertNextValue ( vtkCommand::ModifiedEvent );
          vtkSetAndObserveMRMLNodeEventsMacro(this->ObservedNodes[this->ObservedNodes.size()-1], 
                                              annotationNode, events);
          }
        }
      annotationNodes->Delete();
      }
    else if (transformableNode)
      {
      this->ObservedNodes.push_back(NULL);
      vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
      events->InsertNextValue ( vtkMRMLTransformableNode::TransformModifiedEvent );
      events->InsertNextValue ( vtkMRMLModelNode::PolyDataModifiedEvent );
      events->InsertNextValue ( vtkMRMLVolumeNode::ImageDataModifiedEvent );
      events->InsertNextValue ( vtkCommand::ModifiedEvent );
      vtkSetAndObserveMRMLNodeEventsMacro(this->ObservedNodes[this->ObservedNodes.size()-1], 
                                          transformableNode, events);
      }
    }
  else
    {
    vtkSetAndObserveMRMLNodeMacro(this->DiffusionTensorVolumeNode, NULL);
    }
  return;
}

//----------------------------------------------------------------------------
int vtkSlicerTractographyFiducialSeedingLogic::IsObservedNode(vtkMRMLNode *node)
{
  std::vector<vtkMRMLTransformableNode *>::const_iterator observedNodeIt =
    std::find(this->ObservedNodes.begin(), this->ObservedNodes.end(),node);
  return observedNodeIt != this->ObservedNodes.end();
}

//----------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic::CreateTractsForOneSeed(vtkSeedTracts *seed,
                                                            vtkMRMLDiffusionTensorVolumeNode *volumeNode,
                                                            vtkMRMLTransformableNode *transformableNode,
                                                            int stoppingMode, 
                                                            double stoppingValue, 
                                                            double stoppingCurvature, 
                                                            double integrationStepLength,
                                                            double minPathLength,
                                                            double regionSize, double sampleStep,
                                                            int maxNumberOfSeeds,
                                                            int seedSelectedFiducials)
{
  double sp[3];
  volumeNode->GetSpacing(sp);

  //2. Set Up matrices
  vtkMRMLTransformNode* vxformNode = volumeNode->GetParentTransformNode();
  vtkMRMLTransformNode* fxformNode = transformableNode->GetParentTransformNode();
  vtkMatrix4x4* transformVolumeToFiducial = vtkMatrix4x4::New();
  transformVolumeToFiducial->Identity();
  if (fxformNode != NULL )
    {
    fxformNode->GetMatrixTransformToNode(vxformNode, transformVolumeToFiducial);
    } 
  else if (vxformNode != NULL ) 
    {
    vxformNode->GetMatrixTransformToNode(fxformNode, transformVolumeToFiducial);
    transformVolumeToFiducial->Invert();
    }
  vtkTransform *transFiducial = vtkTransform::New();
  transFiducial->Identity();
  transFiducial->PreMultiply();
  transFiducial->SetMatrix(transformVolumeToFiducial);

  vtkMatrix4x4 *mat = vtkMatrix4x4::New();
  volumeNode->GetRASToIJKMatrix(mat);
  
  vtkMatrix4x4 *TensorRASToIJK = vtkMatrix4x4::New();
  TensorRASToIJK->DeepCopy(mat);
  mat->Delete();


  vtkTransform *trans = vtkTransform::New();
  trans->Identity();
  trans->PreMultiply();
  trans->SetMatrix(TensorRASToIJK);
  // Trans from IJK to RAS
  trans->Inverse();
  // Take into account spacing to compute Scaled IJK
  trans->Scale(1/sp[0],1/sp[1],1/sp[2]);
  trans->Inverse();
  
  //Set Transformation to seeding class
  seed->SetWorldToTensorScaledIJK(trans);
  
  vtkMatrix4x4 *TensorRASToIJKRotation = vtkMatrix4x4::New();
  TensorRASToIJKRotation->DeepCopy(TensorRASToIJK);
  
  //Set Translation to zero
  for (int i=0;i<3;i++)
    {
    TensorRASToIJKRotation->SetElement(i,3,0);
    }
  //Remove scaling in rasToIjk to make a real roation matrix
  double col[3];
  for (int jjj = 0; jjj < 3; jjj++) 
    {
    for (int iii = 0; iii < 3; iii++)
      {
      col[iii]=TensorRASToIJKRotation->GetElement(iii,jjj);
      }
    vtkMath::Normalize(col);
    for (int iii = 0; iii < 3; iii++)
      {
      TensorRASToIJKRotation->SetElement(iii,jjj,col[iii]);
     }  
  }
  TensorRASToIJKRotation->Invert();
  seed->SetTensorRotationMatrix(TensorRASToIJKRotation);  

  //ROI comes from tensor, IJKToRAS is the same
  // as the tensor
  vtkTransform *trans2 = vtkTransform::New();
  trans2->Identity();
  trans2->SetMatrix(TensorRASToIJK);
  trans2->Inverse();
  seed->SetROIToWorld(trans2);
  
  seed->UseVtkHyperStreamlinePoints();
  vtkHyperStreamlineDTMRI *streamer=vtkHyperStreamlineDTMRI::New();
  seed->SetVtkHyperStreamlinePointsSettings(streamer);
  seed->SetMinimumPathLength(minPathLength);

  if ( stoppingMode == 0 )
    {
     streamer->SetStoppingModeToLinearMeasure();
    }
  else if ( stoppingMode == 1 )
    {  
    streamer->SetStoppingModeToFractionalAnisotropy();
    }
  else if ( stoppingMode == 2 )
    {  
    streamer->SetStoppingModeToPlanarMeasure();
    }

  //streamer->SetMaximumPropagationDistance(this->MaximumPropagationDistance);
  streamer->SetStoppingThreshold(stoppingValue);
  streamer->SetRadiusOfCurvature(stoppingCurvature);
  streamer->SetIntegrationStepLength(integrationStepLength);

  // Temp fix to provide a scalar
  seed->GetInputTensorField()->GetPointData()->SetScalars(volumeNode->GetImageData()->GetPointData()->GetScalars());
  
  vtkMRMLAnnotationControlPointsNode *annotationNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(transformableNode);
  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(transformableNode);


  // if annotation
  if (annotationNode && annotationNode->GetNumberOfControlPoints() &&
     (!seedSelectedFiducials || (seedSelectedFiducials && annotationNode->GetSelected())) ) 
    {
    for (int i=0; i < annotationNode->GetNumberOfControlPoints(); i++)
      {
      double *xyzf = annotationNode->GetControlPointCoordinates(i);
      for (double x = -regionSize/2.0; x <= regionSize/2.0; x+=sampleStep)
        {
        for (double y = -regionSize/2.0; y <= regionSize/2.0; y+=sampleStep)
          {
          for (double z = -regionSize/2.0; z <= regionSize/2.0; z+=sampleStep)
            {
            float newXYZ[3];
            newXYZ[0] = xyzf[0] + x;
            newXYZ[1] = xyzf[1] + y;
            newXYZ[2] = xyzf[2] + z;
            float *xyz = transFiducial->TransformFloatPoint(newXYZ);
            //Run the thing
            seed->SeedStreamlineFromPoint(xyz[0], xyz[1], xyz[2]);
            }
          }
        }
      }
    }
  else if (modelNode) 
    {
    this->MaskPoints->SetInput(modelNode->GetPolyData());
    this->MaskPoints->SetRandomMode(1);
    this->MaskPoints->SetMaximumNumberOfPoints(maxNumberOfSeeds);
    this->MaskPoints->Update();
    vtkPolyData *mpoly = this->MaskPoints->GetOutput();

    int nf = mpoly->GetNumberOfPoints();
    for (int f=0; f<nf; f++)
      {
      double *xyzf = mpoly->GetPoint(f);

      double *xyz = transFiducial->TransformDoublePoint(xyzf);
           
      //Run the thing
      seed->SeedStreamlineFromPoint(xyz[0], xyz[1], xyz[2]);
      }
    }

  // Delete everything: Still trying to figure out what is going on

  TensorRASToIJK->Delete();
  TensorRASToIJKRotation->Delete();
  trans2->Delete();
  trans->Delete();
  streamer->Delete();
  transformVolumeToFiducial->Delete();
  transFiducial->Delete();

}




//----------------------------------------------------------------------------
int vtkSlicerTractographyFiducialSeedingLogic::CreateTracts(vtkMRMLTractographyFiducialSeedingNode *parametersNode,
                                                            vtkMRMLDiffusionTensorVolumeNode *volumeNode,
                                                            vtkMRMLNode *seedingyNode,
                                                            vtkMRMLFiberBundleNode *fiberNode,
                                                            int stoppingMode, 
                                                            double stoppingValue, 
                                                            double stoppingCurvature, 
                                                            double integrationStepLength,
                                                            double minPathLength,
                                                            double regionSize, double sampleStep,
                                                            int maxNumberOfSeeds,
                                                            int seedSelectedFiducials,
                                                            int displayMode)
{
  // 0. check inputs
  if (volumeNode == NULL || seedingyNode == NULL || fiberNode == NULL ||
      volumeNode->GetImageData() == NULL)
    {
    if (fiberNode && fiberNode->GetPolyData())
      {
      fiberNode->GetPolyData()->Reset();
      }
    return 0;
    }
    
  vtkPolyData *oldPoly = fiberNode->GetPolyData();
   
  vtkSeedTracts *seed = vtkSeedTracts::New();

  
  //1. Set Input

  vtkMRMLTransformNode* vxformNode = volumeNode->GetParentTransformNode();

  vtkMRMLAnnotationHierarchyNode *annotationListNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(seedingyNode);
  vtkMRMLAnnotationControlPointsNode *annotationNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(seedingyNode);
  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(seedingyNode);
  vtkMRMLScalarVolumeNode *labelMapNode = vtkMRMLScalarVolumeNode::SafeDownCast(seedingyNode);

   if( parametersNode->GetWriteToFile() )
    {
    seed->SetFileDirectoryName(parametersNode->GetFileDirectoryName());
    if( parametersNode->GetFilePrefix() != 0 )
      {
      seed->SetFilePrefix(parametersNode->GetFilePrefix() );
      }
    }


  //Do scale IJK
  double sp[3];
  volumeNode->GetSpacing(sp);
  vtkImageChangeInformation *ici = vtkImageChangeInformation::New();
  ici->SetOutputSpacing(sp);
  ici->SetInput(volumeNode->GetImageData());
  ici->GetOutput()->Update();

  seed->SetInputTensorField(ici->GetOutput());

  if (labelMapNode)
    {
    
    this->CreateTractsForLabelMap(seed, volumeNode, labelMapNode,
                                  parametersNode->GetROILabel(),
                                  parametersNode->GetUseIndexSpace(),
                                  parametersNode->GetSeedSpacing(),
                                  parametersNode->GetRandomGrid(),
                                  parametersNode->GetLinearMeasureStart(),
                                  parametersNode->GetStoppingMode(), 
                                  parametersNode->GetStoppingValue(), 
                                  parametersNode->GetStoppingCurvature(), 
                                  parametersNode->GetIntegrationStep(),
                                  parametersNode->GetMinimumPathLength(),
                                  parametersNode->GetMaximumPathLength());
    }
  else if (annotationListNode) // loop over annotation nodes
    {
    vtkCollection *annotationNodes = vtkCollection::New();
    annotationListNode->GetDirectChildren(annotationNodes);
    int nf = annotationNodes->GetNumberOfItems();
    for (int f=0; f<nf; f++)
      {
      vtkMRMLAnnotationControlPointsNode *annotationNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(
                                                    annotationNodes->GetItemAsObject(f));
      if (!annotationNode || (seedSelectedFiducials && !annotationNode->GetSelected()))
        {
        continue;
        }


      this->CreateTractsForOneSeed(seed, volumeNode, annotationNode, 
                                   stoppingMode, stoppingValue, stoppingCurvature, 
                                   integrationStepLength, minPathLength, regionSize, 
                                   sampleStep, maxNumberOfSeeds, seedSelectedFiducials);

      }
      annotationNodes->Delete();
    }
  else if (annotationNode) // loop over points in the models
    {
    this->CreateTractsForOneSeed(seed, volumeNode, annotationNode,
                                 stoppingMode, stoppingValue, stoppingCurvature, 
                                 integrationStepLength, minPathLength, regionSize, 
                                 sampleStep, maxNumberOfSeeds, seedSelectedFiducials);

    }

  else if (modelNode) // loop over points in the models
    {
    this->CreateTractsForOneSeed(seed, volumeNode, modelNode,
                                 stoppingMode, stoppingValue, stoppingCurvature, 
                                 integrationStepLength, minPathLength, regionSize, 
                                 sampleStep, maxNumberOfSeeds, seedSelectedFiducials);

    }
  ici->Delete();
  
  //6. Extra5ct PolyData in RAS
  if( !parametersNode->GetWriteToFile()  )
  {
    vtkPolyData *outFibers = vtkPolyData::New();
    
    seed->TransformStreamlinesToRASAndAppendToPolyData(outFibers);

    fiberNode->SetAndObservePolyData(outFibers);

    //For the results to reflect the paremeters, we make sure that there is no subsampling in the fibers
    fiberNode->SetSubsamplingRatio(1.);
    
    int newNode = 0;
    vtkMRMLFiberBundleDisplayNode *dnode = fiberNode->GetTubeDisplayNode();
    if (dnode == NULL || oldPoly == NULL)
      {
      dnode = fiberNode->AddTubeDisplayNode();
      dnode->DisableModifiedEventOn();
      dnode->SetScalarVisibility(1);
      dnode->SetOpacity(1);
      dnode->SetVisibility(1);
      dnode->DisableModifiedEventOff();
      newNode = 1;
      }
    if (oldPoly == NULL && displayMode == 1)
      {
      dnode->SetVisibility(1);
      }
    else if (oldPoly == NULL && displayMode == 0)
      {
      dnode->SetVisibility(0);
      }

    dnode = fiberNode->GetLineDisplayNode();
    if (dnode == NULL || oldPoly == NULL)
      {
      dnode = fiberNode->AddLineDisplayNode();
      if (newNode)
        {
        dnode->DisableModifiedEventOn();
        dnode->SetVisibility(0);
        dnode->SetOpacity(1);
        dnode->SetScalarVisibility(0);
        dnode->DisableModifiedEventOff();
        }
      }
    if (oldPoly == NULL && displayMode == 0)
      {
      dnode->SetVisibility(1);
      }
    else if (oldPoly == NULL && displayMode == 1)
      {
      dnode->SetVisibility(0);
      }

    dnode = fiberNode->GetGlyphDisplayNode();
    if (dnode == NULL || oldPoly == NULL)
      {
      dnode = fiberNode->AddGlyphDisplayNode();
      if (newNode)
        {
        dnode->DisableModifiedEventOn();
        dnode->SetVisibility(0);
        dnode->SetScalarVisibility(1);
        dnode->SetOpacity(1);
        dnode->DisableModifiedEventOff();
        }
      }


    if (fiberNode->GetStorageNode() == NULL) 
      {
      vtkMRMLFiberBundleStorageNode *storageNode = vtkMRMLFiberBundleStorageNode::New();
      fiberNode->GetScene()->AddNode(storageNode);
      fiberNode->SetAndObserveStorageNodeID(storageNode->GetID());
      storageNode->Delete();
      }

    fiberNode->InvokeEvent(vtkMRMLFiberBundleNode::PolyDataModifiedEvent, NULL);
   
    if (vxformNode != NULL )
      { 
      fiberNode->SetAndObserveTransformNodeID(vxformNode->GetID());
      } 
    else 
      {
      fiberNode->SetAndObserveTransformNodeID(NULL);
      }
  }

  // Delete everything: Still trying to figure out what is going on
  seed->Delete();
  return 1;
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic::ProcessMRMLNodesEvents(vtkObject *caller,
                                                                      unsigned long event,
                                                                      void *callData) 
{

  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
    {
    return;
    }

  vtkMRMLTractographyFiducialSeedingNode* snode = this->TractographyFiducialSeedingNode;

  if (snode == NULL || snode->GetEnableSeeding() == 0)
    {
    return;
    }

  if (event == vtkMRMLHierarchyNode::ChildNodeAddedEvent ||
      event == vtkMRMLHierarchyNode::ChildNodeRemovedEvent ||
      event == vtkMRMLNode::HierarchyModifiedEvent || 
      event == vtkMRMLTransformableNode::TransformModifiedEvent ||
      event == vtkMRMLModelNode::PolyDataModifiedEvent)
    {
    this->OnMRMLNodeModified(NULL);
    }    
  else
    {
    Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic::OnMRMLSceneEndImport()
{
  // if we have a parameter node select it
  vtkMRMLTractographyFiducialSeedingNode *tnode = 0;
  vtkMRMLNode *node = this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLTractographyFiducialSeedingNode");
  if (node)
    {
    tnode = vtkMRMLTractographyFiducialSeedingNode::SafeDownCast(node);
    vtkSetAndObserveMRMLNodeMacro(this->TractographyFiducialSeedingNode, tnode);
    this->RemoveMRMLNodesObservers();
    this->AddMRMLNodesObservers();
    }
  this->InvokeEvent(vtkMRMLScene::EndImportEvent);
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (node == NULL || !this->IsObservedNode(node))
    {      
    return;
    }
  this->OnMRMLNodeModified(node);
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic
::OnMRMLNodeModified(vtkMRMLNode* vtkNotUsed(node))
{
  vtkMRMLTractographyFiducialSeedingNode* snode = this->TractographyFiducialSeedingNode;

  if (snode == NULL || snode->GetEnableSeeding() == 0)
    {
    return;
    }

  this->RemoveMRMLNodesObservers();

  this->AddMRMLNodesObservers();

  vtkMRMLDiffusionTensorVolumeNode *volumeNode =
    vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(snode->GetInputVolumeRef()));
  vtkMRMLNode *seedingNode = this->GetMRMLScene()->GetNodeByID(snode->GetInputFiducialRef());
  vtkMRMLFiberBundleNode *fiberNode = 
    vtkMRMLFiberBundleNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(snode->GetOutputFiberRef()));

  if(volumeNode == NULL || seedingNode == NULL || fiberNode == NULL) 
    {
    return;
    }
  
  this->CreateTracts(snode, volumeNode, seedingNode, fiberNode, 
                     snode->GetStoppingMode(), 
                     snode->GetStoppingValue(),
                     snode->GetStoppingCurvature(),
                     snode->GetIntegrationStep(),
                     snode->GetMinimumPathLength(),
                     snode->GetSeedingRegionSize(),
                     snode->GetSeedingRegionStep(),
                     snode->GetMaxNumberOfSeeds(),
                     snode->GetSeedSelectedFiducials(),
                     snode->GetDisplayMode()
                     );
}

//----------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
    {
    return;
    }
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLTractographyFiducialSeedingNode>::New());
}

//---------------------------------------------------------------------------
// Set the internal mrml scene adn observe events on it
//---------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events);
  events->Delete();
}

//----------------------------------------------------------------------------
int vtkSlicerTractographyFiducialSeedingLogic::CreateTractsForLabelMap(
                                                            vtkSeedTracts *seed,
                                                            vtkMRMLDiffusionTensorVolumeNode *volumeNode,
                                                            vtkMRMLScalarVolumeNode *seedingNode,
                                                            int ROIlabel,
                                                            int useIndexSpace,
                                                            double seedSpacing,
                                                            int randomGrid,
                                                            double linearMeasureStart,
                                                            int stoppingMode, 
                                                            double stoppingValue, 
                                                            double stoppingCurvature, 
                                                            double integrationStepLength,
                                                            double minPathLength,
                                                            double maxPathLength)
{
  if( volumeNode->GetImageData()->GetPointData()->GetTensors() == NULL )
    {
    vtkErrorMacro("No tensor data");
    return 0;
    }

  if( seedingNode->GetImageData()->GetPointData()->GetScalars() == NULL )
    {
    vtkErrorMacro("No label data");
    return 0;
    }

  vtkSmartPointer<vtkImageData> ROI;
  vtkSmartPointer<vtkImageCast> imageCast = vtkSmartPointer<vtkImageCast>::New();
  vtkSmartPointer<vtkDiffusionTensorMathematics> math = vtkSmartPointer<vtkDiffusionTensorMathematics>::New();
  vtkSmartPointer<vtkImageThreshold> th = vtkSmartPointer<vtkImageThreshold>::New();
  vtkSmartPointer<vtkMatrix4x4> ROIRASToIJK = vtkSmartPointer<vtkMatrix4x4>::New();
  
  // 1. Set Input

  if (seedingNode->GetImageData())
    {
    // cast roi to short data type
    imageCast->SetOutputScalarTypeToShort();
    imageCast->SetInput(seedingNode->GetImageData() );
    imageCast->Update();

    //Do scale IJK
    double sp[3];
    seedingNode->GetSpacing(sp);
    vtkImageChangeInformation *ici = vtkImageChangeInformation::New();
    ici->SetOutputSpacing(sp);
    ici->SetInput(imageCast->GetOutput());
    ici->GetOutput()->Update();

    ROI = ici->GetOutput();

    // Set up the matrix that will take points in ROI
    // to RAS space.  Code assumes this is world space
    // since  we have no access to external transforms.
    // This will only work if no transform is applied to
    // ROI and tensor volumes.
    //
    seedingNode->GetRASToIJKMatrix(ROIRASToIJK) ;
    } 
  else 
    {
    math->SetInput(0, volumeNode->GetImageData());
    if ( stoppingMode == 0 )
      {
      math->SetOperationToLinearMeasure();
      }
    else
      {  
      math->SetOperationToFractionalAnisotropy();
      }
    math->Update();

    th->SetInput(math->GetOutput());
    th->ThresholdBetween(linearMeasureStart,1);
    th->SetInValue(ROIlabel);
    th->SetOutValue(0);
    th->ReplaceInOn();
    th->ReplaceOutOn();
    th->SetOutputScalarTypeToShort();
    th->Update();
    ROI = th->GetOutput();

    // Set up the matrix that will take points in ROI
    // to RAS space.  Code assumes this is world space
    // since  we have no access to external transforms.
    // This will only work if no transform is applied to
    // ROI and tensor volumes.
    volumeNode->GetRASToIJKMatrix(ROIRASToIJK) ;
  }

  // 2. Set Up matrices
  vtkSmartPointer<vtkMatrix4x4> TensorRASToIJK = vtkSmartPointer<vtkMatrix4x4>::New();
  
  volumeNode->GetRASToIJKMatrix(TensorRASToIJK);

  // VTK seeding is in ijk space with voxel scale included.
  // Calculate the matrix that goes from tensor "scaled IJK",
  // the array with voxels that know their size (what vtk sees for tract seeding)
  // to our RAS.
  double sp[3];
  volumeNode->GetSpacing(sp);
  vtkSmartPointer<vtkTransform> trans = vtkSmartPointer<vtkTransform>::New();
  trans->Identity();
  trans->PreMultiply();
  trans->SetMatrix(TensorRASToIJK);
  // Trans from IJK to RAS
  trans->Inverse();
  // Take into account spacing (remove from matrix) to compute Scaled IJK to RAS matrix
  trans->Scale(1 / sp[0], 1 / sp[1], 1 / sp[2]);
  trans->Inverse();

  // Set Transformation to seeding class
  seed->SetWorldToTensorScaledIJK(trans);

  // Rotation part of matrix is only thing tensor is transformed by.
  // This is to transform output tensors into RAS space.
  // Tensors are output along the fibers.
  // This matrix is not used for calculating tractography.
  // The following should be replaced with finite strain method
  // rather than assuming rotation part of the matrix according to
  // slicer convention.
  vtkSmartPointer<vtkMatrix4x4> TensorRASToIJKRotation = vtkSmartPointer<vtkMatrix4x4>::New();
  TensorRASToIJKRotation->DeepCopy(TensorRASToIJK);
  // Set Translation to zero
  for( int i = 0; i < 3; i++ )
    {
    TensorRASToIJKRotation->SetElement(i, 3, 0);
    }
  // Remove scaling in rasToIjk to make a real rotation matrix
  double col[3];
  for( int jjj = 0; jjj < 3; jjj++ )
    {
    for( int iii = 0; iii < 3; iii++ )
      {
      col[iii] = TensorRASToIJKRotation->GetElement(iii, jjj);
      }
    vtkMath::Normalize(col);
    for( int iii = 0; iii < 3; iii++ )
      {
      TensorRASToIJKRotation->SetElement(iii, jjj, col[iii]);
      }
    }
  TensorRASToIJKRotation->Invert();
  seed->SetTensorRotationMatrix(TensorRASToIJKRotation);

  // vtkSmartPointer<vtkNRRDWriter> iwriter = vtkSmartPointer<vtkNRRDWriter>::New();

  // 3. Set up ROI (not based on Cl mask), from input now


  // Create Cl mask
  /**
  iwriter->SetInput(imageCast->GetOutput());
  iwriter->SetFileName("C:/Temp/cast.nhdr");
  iwriter->Write();


  vtkSmartPointer<vtkDiffusionTensorMathematicsSimple> math = vtkSmartPointer<vtkDiffusionTensorMathematicsSimple>::New();
  math->SetInput(0, volumeNode->GetImageData());
  // math->SetInput(1, volumeNode->GetImageData());
  math->SetScalarMask(imageCast->GetOutput());
  math->MaskWithScalarsOn();
  math->SetMaskLabelValue(ROIlabel);
  math->SetOperationToLinearMeasure();
  math->Update();

  iwriter->SetInput(math->GetOutput());
  iwriter->SetFileName("C:/Temp/math.nhdr");
  iwriter->Write();

  vtkSmartPointer<vtkImageThreshold> th = vtkSmartPointer<vtkImageThreshold>::New();
  th->SetInput(math->GetOutput());
  th->ThresholdBetween(linearMeasureStart,1);
  th->SetInValue(1);
  th->SetOutValue(0);
  th->ReplaceInOn();
  th->ReplaceOutOn();
  th->SetOutputScalarTypeToShort();
  th->Update();

  iwriter->SetInput(th->GetOutput());
  iwriter->SetFileName("C:/Temp/th.nhdr");
  iwriter->Write();
  **/

  vtkSmartPointer<vtkTransform> trans2 = vtkSmartPointer<vtkTransform>::New();
  trans2->Identity();
  trans2->PreMultiply();

  // no longer assume this ROI is in tensor space
  // trans2->SetMatrix(TensorRASToIJK);
  trans2->SetMatrix(ROIRASToIJK);
  trans2->Inverse();
  seed->SetROIToWorld(trans2);


  // PENDING: Do merging with input ROI

  seed->SetInputROI(ROI);
  seed->SetInputROIValue(ROIlabel);
  seed->UseStartingThresholdOn();
  seed->SetStartingThreshold(linearMeasureStart);


  // 4. Set Tractography specific parameters

  if( useIndexSpace )
    {
    seed->SetIsotropicSeeding(0);
    }
  else
    {
    seed->SetIsotropicSeeding(1);
    }

  seed->SetRandomGrid(randomGrid);

  seed->SetIsotropicSeedingResolution(seedSpacing);
  seed->SetMinimumPathLength(minPathLength);
  seed->UseVtkHyperStreamlinePoints();
  vtkSmartPointer<vtkHyperStreamlineDTMRI> streamer = vtkSmartPointer<vtkHyperStreamlineDTMRI>::New();
  seed->SetVtkHyperStreamlinePointsSettings(streamer);

  if ( stoppingMode == 0 )
    {
     streamer->SetStoppingModeToLinearMeasure();
    }
  else if ( stoppingMode == 1 )
    {  
    streamer->SetStoppingModeToFractionalAnisotropy();
    }
  else if ( stoppingMode == 2 )
    {  
    streamer->SetStoppingModeToPlanarMeasure();
    }

  streamer->SetStoppingThreshold(stoppingValue);
  streamer->SetMaximumPropagationDistance(maxPathLength);
  streamer->SetRadiusOfCurvature(stoppingCurvature);
  streamer->SetIntegrationStepLength(integrationStepLength);

  // Temp fix to provide a scalar
  // seed->GetInputTensorField()->GetPointData()->SetScalars(math->GetOutput()->GetPointData()->GetScalars());

  // 5. Run the thing
  seed->SeedStreamlinesInROI();

  return 1;
}
