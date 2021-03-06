
#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationControlPointsStorageNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationFiducialsStorageNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationHierarchyStorageNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationRulerStorageNode.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationStorageNode.h"
#include "vtkMRMLHierarchyNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"

#include <vtkMRMLDisplayableHierarchyLogic.h>

#include "vtkStringArray.h"

#include <fstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationHierarchyStorageNode);

//----------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyStorageNode::vtkMRMLAnnotationHierarchyStorageNode()
{
  //this->Debug =1;
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyStorageNode::~vtkMRMLAnnotationHierarchyStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLAnnotationHierarchyStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLAnnotationHierarchyNode");
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationHierarchyStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  // cast the input node
  vtkMRMLAnnotationHierarchyNode *annotationHierarchyNode =
    vtkMRMLAnnotationHierarchyNode::SafeDownCast(refNode);

  if (annotationHierarchyNode == NULL)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a annotation hierarchy node");
    return 0;
    }

  // clear out the list
//  annotationHierarchyNode->RemoveAllHierarchyChildrenNodes();

  // READ
  std::string fullName = this->GetFullNameFromFileName(); 

  if (fullName == std::string("")) 
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  // open the file
  fstream fstr;
  fstr.open(fullName.c_str(), fstream::in);
  if (!fstr.is_open())
    {
    vtkErrorMacro("ReadData: unable to open file " << fullName.c_str() << " for reading");
    return 0;
    }

  // turn on batch processing
  this->GetScene()->StartState(vtkMRMLScene::BatchProcessState);
  
  char line[1024];
  while (fstr.good())
    {
    fstr.getline(line, 1024);
    // parse line:
    // # New Annotation: vtkMRMLAnnotationXNode
    // to figure out what kind of node to instantiate
    if (strncmp(line, "# New Annotation: ", 17) == 0)
      {
      std::string linestr = std::string(line);
      std::string className = linestr.substr(18, std::string::npos);
//      std::cout << "Got className = '" << className.c_str() << "'" << std::endl;
      if (className.compare("vtkMRMLAnnotationFiducialNode") == 0)
        {
        vtkDebugMacro("ReadData: have a fiducial node");
        vtkSmartPointer<vtkMRMLAnnotationFiducialsStorageNode> fStorageNode = vtkSmartPointer<vtkMRMLAnnotationFiducialsStorageNode>::New();
        vtkMRMLAnnotationFiducialNode * fnode = vtkMRMLAnnotationFiducialNode::New();
        // add the storage node to the scene
        this->GetScene()->AddNode(fStorageNode);
        fnode->SetScene(this->GetScene());
        this->GetScene()->AddNode(fnode);
        fnode->SetAndObserveStorageNodeID(fStorageNode->GetID());
        // call the method that will work with an open file pointer and return
        // when done reading one fid
        if (fStorageNode->ReadOneFiducial(fstr, fnode))
          {
          vtkDebugMacro("ReadData: read a fiducial from the file " << fullName.c_str());
          // set the parent to be this hierarchy
          vtkMRMLHierarchyNode *fidHierarchyNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(fnode->GetScene(), fnode->GetID());
          if (fidHierarchyNode)
            {
//            std::cout << "Found a hierarchy node for new fiducial: " << fidHierarchyNode->GetName() << ", setting it's parent node id to " << refNode->GetID() << std::endl;
            fidHierarchyNode->SetParentNodeID(refNode->GetID());
            }
          else
            {
            vtkWarningMacro("ReadData: didn't find a hierarchy node for new fiducial " << fnode->GetName() << ", adding one");
            // add one
            vtkMRMLDisplayableHierarchyLogic *displayableHierarchyLogic = vtkMRMLDisplayableHierarchyLogic::New();
            char *hid = displayableHierarchyLogic->AddDisplayableHierarchyNodeForNode(fnode);
            if (hid)
              {
              fidHierarchyNode =  vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(fnode->GetScene(), fnode->GetID());
              if (fidHierarchyNode)
                {
                fidHierarchyNode->SetParentNodeID(refNode->GetID());
                }
              else
                {
                vtkErrorMacro("ReadData: unable to create a hierarchy node for fiducial " << fnode->GetName());
                }
              }
            displayableHierarchyLogic->Delete();
            }
          fnode->Delete();
          }
        else
          {
          vtkErrorMacro("ReadData: failed to read a fiducial from the file " << fullName.c_str());
          }
        }
      else if (className.compare("vtkMRMLAnnotationRulerNode") == 0)
        {
        vtkWarningMacro("ReadData: have a ruler node, saved in a separate file");
        /*
        vtkSmartPointer<vtkMRMLAnnotationRulerStorageNode> rStorageNode = vtkSmartPointer<vtkMRMLAnnotationRulerStorageNode>::New();
        vtkMRMLAnnotationRulerNode * rnode = vtkMRMLAnnotationRulerNode::New();
        // add the storage node to the scene
        this->GetScene()->AddNode(rStorageNode);
        rnode->SetScene(this->GetScene());
        this->GetScene()->AddNode(rnode);
        rnode->SetAndObserveStorageNodeID(rStorageNode->GetID());
        // call the method that will work with an open file pointer and return
        // when done reading one ruler
        if (rStorageNode->ReadOneRuler(fstr, rnode))
          {
          vtkDebugMacro("ReadData: read a ruler from the file " << fullName.c_str());
          }
        else
          {
          vtkErrorMacro("ReadData: failed to read a ruler from the file " << fullName.c_str());
          }
        */
        }
      else
        {
        vtkWarningMacro("ReadData: unknown type of annotation, can't read it: '" << className.c_str() << "'");
        }
      }
    }
  // turn off batch processing
  this->GetScene()->EndState(vtkMRMLScene::BatchProcessState);
  
  // close the file
  fstr.close();

  // mark it unmodified since read
  this->InvokeEvent(vtkMRMLScene::NodeAddedEvent, annotationHierarchyNode);

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationHierarchyStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  if (!refNode)
    {
    vtkWarningMacro("WriteData: reference node is null.");
    return 0;
    }

  // is the correct kind of node?
  if (!refNode->IsA("vtkMRMLAnnotationHierarchyNode"))
    {
    vtkErrorMacro("WriteData: Reference node is not a vtkMRMLAnnotationHierarchyNode");
    return 0;
    }
  
  vtkMRMLAnnotationHierarchyNode *hNode =
    vtkMRMLAnnotationHierarchyNode::SafeDownCast(refNode);

  std::string fullName = this->GetFullNameFromFileName();  
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("WriteData: File name not specified");
    return 0;
    }
  
  std::vector< vtkMRMLHierarchyNode* > children = hNode->GetChildrenNodes();
  fstream of;
  if (children.size())
    {
    // open file for writing
    of.open(fullName.c_str(), fstream::out);
    if (!of.is_open())
      {
      vtkErrorMacro("WriteData: unable to open file " << fullName.c_str() << " for writing");
      return 0;
      }
    of << "# Annotation Hierarchy File " << (this->GetFileName() != NULL ? this->GetFileName() : "null") << endl;
    }
  for (unsigned int i=0; i<children.size(); i++)
    {
    vtkMRMLAnnotationHierarchyNode *child = vtkMRMLAnnotationHierarchyNode::SafeDownCast(children[i]);
    if (child)
      {
      vtkMRMLDisplayableNode *dnode = child->GetDisplayableNode();
      if (dnode && dnode->IsA("vtkMRMLAnnotationNode"))
        {
        vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(dnode);
        vtkDebugMacro("WriteData: have an annotation node with name " << annotationNode->GetName());
        // separate this annotation by a comment line 
        //of << "# New Annotation: " << annotationNode->GetClassName() << endl;
        // get it's storage node and use the write to buffer methods
        vtkMRMLStorageNode *storageNode = annotationNode->GetStorageNode();
        if (!storageNode && this->GetScene())
          {
          vtkWarningMacro("Creating a new storage node for " << annotationNode->GetName());
          vtkMRMLStorageNode *newStorageNode = annotationNode->CreateDefaultStorageNode();
          this->GetScene()->AddNode(newStorageNode);
          annotationNode->SetAndObserveStorageNodeID(newStorageNode->GetID());
          newStorageNode->Delete();
          storageNode = newStorageNode;
          }
        vtkMRMLAnnotationStorageNode *annotationStorageNode = NULL;
        if (storageNode)
          {
          annotationStorageNode = vtkMRMLAnnotationStorageNode::SafeDownCast(storageNode);
          }
        else
          {
          vtkWarningMacro("WriteData: no storage node for annotation node " << annotationNode->GetName());
          }
        if (annotationStorageNode)
          {
          if (annotationStorageNode->IsA("vtkMRMLAnnotationRulerStorageNode"))
            {
            /* disable for now, as can't read it yet
            of << "# New Annotation: " << annotationNode->GetClassName() << endl;
            vtkMRMLAnnotationRulerStorageNode *rulerStorageNode = vtkMRMLAnnotationRulerStorageNode::SafeDownCast(annotationStorageNode);
            int retval = rulerStorageNode->WriteData(annotationNode, of);
            if (!retval)
              {
              vtkErrorMacro("Error writing data for ruler annotation " << annotationNode->GetName());
              }
            */
            }
          else if (annotationStorageNode->IsA("vtkMRMLAnnotationFiducialsStorageNode"))
            {
             // separate this annotation by a comment line 
            of << "# New Annotation: " << annotationNode->GetClassName() << endl;
            vtkMRMLAnnotationFiducialsStorageNode *fidStorageNode = vtkMRMLAnnotationFiducialsStorageNode::SafeDownCast(annotationStorageNode);
            int retval = fidStorageNode->WriteDataInternal(annotationNode, of);
            if (!retval)
              {
              vtkErrorMacro("Error writing data for fiducial annotation " << annotationNode->GetName());
              }
            }
          else if (annotationStorageNode->IsA("vtkMRMLAnnotationControlPointsStorageNode"))
            {
            of << "# New Annotation: " << annotationNode->GetClassName() << endl;
            vtkMRMLAnnotationControlPointsStorageNode *cpStorageNode = vtkMRMLAnnotationControlPointsStorageNode::SafeDownCast(annotationStorageNode);
            int retval = cpStorageNode->WriteDataInternal(annotationNode, of);
            if (!retval)
              {
              vtkErrorMacro("Error writing data for fiducial annotation " << annotationNode->GetName());
              }
            }
          else
            {
            std::cout << "Unknown storage node class type: " << annotationStorageNode->GetClassName() << std::endl;
            }
          }
        else
          {
          vtkWarningMacro("WriteData: no annotation storage node for annotation " << annotationNode->GetName());
          }
        }
      }
    }

  of.close();

  return 1;
}

