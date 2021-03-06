/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2007 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Description

Diffuse wall where oncoming oxygen and nitrogen atoms are catalysed at the
surface to oxygen and nitrogen molecules.

\*---------------------------------------------------------------------------*/

#include "dsmcDiffuseCatalyticWallPatch.H"
#include "addToRunTimeSelectionTable.H"
#include "fvc.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{

defineTypeNameAndDebug(dsmcDiffuseCatalyticWallPatch, 0);

addToRunTimeSelectionTable
(dsmcPatchBoundary, dsmcDiffuseCatalyticWallPatch, dictionary);



// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from components
dsmcDiffuseCatalyticWallPatch::dsmcDiffuseCatalyticWallPatch
(
    Time& t,
    const polyMesh& mesh,
    dsmcCloud& cloud,
    const dictionary& dict
)
:
    dsmcPatchBoundary(t, mesh, cloud, dict),
    propsDict_(dict.subDict(typeName + "Properties")),
    catalysisTypeIds_(),
    catalysedTypeIds_(),
    heatOfReaction_()
{
    writeInTimeDir_ = false;
    writeInCase_ = false;
    measurePropertiesAtWall_ = true;

    setProperties();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

dsmcDiffuseCatalyticWallPatch::~dsmcDiffuseCatalyticWallPatch()
{}



// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //
void dsmcDiffuseCatalyticWallPatch::initialConfiguration()
{
    
}

void dsmcDiffuseCatalyticWallPatch::calculateProperties()
{

}

void dsmcDiffuseCatalyticWallPatch::controlParticle
(dsmcParcel& p, dsmcParcel::trackingData& td)
{
    measurePropertiesBeforeControl(p);

    vector& U = p.U();

    scalar& ERot = p.ERot();
    
    labelList& vibLevel = p.vibLevel();
    
    label& ELevel = p.ELevel();

    label typeId = p.typeId();
    
    label iD = findIndex(catalysisTypeIds_, typeId);
    
    if(iD == -1)
    {       
        vector nw = p.normal();
        nw /= mag(nw);

        // Normal velocity magnitude
        scalar U_dot_nw = U & nw;

        // Wall tangential velocity (flow direction)
        vector Ut = U - U_dot_nw*nw;

        Random& rndGen(cloud_.rndGen());

        while (mag(Ut) < SMALL)
        {
            // If the incident velocity is parallel to the face normal, no
            // tangential direction can be chosen.  Add a perturbation to the
            // incoming velocity and recalculate.

            U = vector
            (
                U.x()*(0.8 + 0.2*rndGen.scalar01()),
                U.y()*(0.8 + 0.2*rndGen.scalar01()),
                U.z()*(0.8 + 0.2*rndGen.scalar01())
            );

            U_dot_nw = U & nw;

            Ut = U - U_dot_nw*nw;
        }

        // Wall tangential unit vector
        vector tw1 = Ut/mag(Ut);

        // Other tangential unit vector
        vector tw2 = nw^tw1;

        const scalar& T = temperature_;

        scalar mass = cloud_.constProps(typeId).mass();

        scalar rotationalDof = 
                cloud_.constProps(typeId).rotationalDegreesOfFreedom();
        
        scalar vibrationalDof = 
                cloud_.constProps(typeId).vibrationalDegreesOfFreedom();
        
        List<label> degeneracyList = 
                cloud_.constProps(typeId).degeneracyList();
        
        List<scalar> electronicEnergyList = 
                cloud_.constProps(typeId).electronicEnergyList();

        U =
            sqrt(physicoChemical::k.value()*T/mass)
        *(
                rndGen.GaussNormal()*tw1
            + rndGen.GaussNormal()*tw2
            - sqrt(-2.0*log(max(1 - rndGen.scalar01(), VSMALL)))*nw
            );

        U += velocity_;

        ERot = cloud_.equipartitionRotationalEnergy(T, rotationalDof);
        
        vibLevel = 
          cloud_.equipartitionVibrationalEnergyLevel(T, vibrationalDof, typeId);
        
        ELevel = cloud_.equipartitionElectronicLevel
                        (T, degeneracyList, electronicEnergyList, typeId);

        measurePropertiesAfterControl(p, 0.0);
    }
    else
    {          
        vector nw = p.normal();
        nw /= mag(nw);

        // Normal velocity magnitude
        scalar U_dot_nw = U & nw;

        // Wall tangential velocity (flow direction)
        vector Ut = U - U_dot_nw*nw;

        Random& rndGen(cloud_.rndGen());

        while (mag(Ut) < SMALL)
        {
            // If the incident velocity is parallel to the face normal, no
            // tangential direction can be chosen.  Add a perturbation to the
            // incoming velocity and recalculate.

            U = vector
            (
                U.x()*(0.8 + 0.2*rndGen.scalar01()),
                U.y()*(0.8 + 0.2*rndGen.scalar01()),
                U.z()*(0.8 + 0.2*rndGen.scalar01())
            );

            U_dot_nw = U & nw;

            Ut = U - U_dot_nw*nw;
        }
        
        // Wall tangential unit vector
        vector tw1 = Ut/mag(Ut);

        // Other tangential unit vector
        vector tw2 = nw^tw1;

        const scalar& T = temperature_;
        
        p.typeId() = catalysedTypeIds_[iD];
        label newTypeId = catalysedTypeIds_[iD];

        scalar mass = cloud_.constProps(newTypeId).mass();

        label rotationalDof = 
                cloud_.constProps(newTypeId).rotationalDegreesOfFreedom();
        
        label vibrationalDof = 
                cloud_.constProps(newTypeId).vibrationalDegreesOfFreedom();
        
        List<label> degeneracyList = 
                cloud_.constProps(newTypeId).degeneracyList();
        
        List<scalar> electronicEnergyList = 
                cloud_.constProps(newTypeId).electronicEnergyList();

        U =
            sqrt(physicoChemical::k.value()*T/mass)
        *(
                rndGen.GaussNormal()*tw1
            + rndGen.GaussNormal()*tw2
            - sqrt(-2.0*log(max(1 - rndGen.scalar01(), VSMALL)))*nw
            );

        U += velocity_;

        ERot = cloud_.equipartitionRotationalEnergy(T, rotationalDof);
        
        vibLevel = cloud_.equipartitionVibrationalEnergyLevel
                                        (T, vibrationalDof, newTypeId);
        
        ELevel = cloud_.equipartitionElectronicLevel
                    (T, degeneracyList, electronicEnergyList, newTypeId);

        measurePropertiesAfterControl(p, heatOfReaction_[iD]);
    }
}

void dsmcDiffuseCatalyticWallPatch::output
(
    const fileName& fixedPathName,
    const fileName& timePath
)
{
}


void dsmcDiffuseCatalyticWallPatch::updateProperties(const dictionary& newDict)
{
    //- the main properties should be updated first
    updateBoundaryProperties(newDict);

    propsDict_ = newDict.subDict(typeName + "Properties");

    setProperties();

}

void dsmcDiffuseCatalyticWallPatch::setProperties()
{
    velocity_ = propsDict_.lookup("velocity");
    temperature_ = readScalar(propsDict_.lookup("temperature"));
    
    // set the molecules/atoms to be catalysed typeIds ------------    
    const List<word> inputMolecules 
            (propsDict_.lookup("moleculesToBeCatalysed"));
            
    if(inputMolecules.size() == 0)
    {
        
        FatalErrorIn("dsmcDiffuseCatalyticWallPatch::setProperties()")
            << "Cannot have zero typeIds being catalysed." << nl << "in: "
            << mesh_.time().system()/"boundariesDict"
            << exit(FatalError);
    }
    
    DynamicList<word> inputMoleculesReduced(0);

    forAll(inputMolecules, i)
    {
        const word& moleculeName(inputMolecules[i]);

        if(findIndex(inputMoleculesReduced, moleculeName) == -1)
        {
            inputMoleculesReduced.append(moleculeName);
        }
    }

    inputMoleculesReduced.shrink();
    
    //  set the type ids
    
    catalysisTypeIds_.setSize(inputMoleculesReduced.size(), -1); 
    
    forAll(inputMoleculesReduced, i)
    {
        const word& moleculeName(inputMoleculesReduced[i]);
        
        label typeId = findIndex(cloud_.typeIdList(), moleculeName);
        
        // check that input molecules belong to the typeIdList
        if(typeId == -1)
        {
            FatalErrorIn("dsmcDiffuseCatalyticWallPatch::setProperties()")
                << "Cannot find type id: " << moleculeName << nl 
                << exit(FatalError);
        }
        
        catalysisTypeIds_[i] = typeId;
    }
    
    // set the product molecules/atoms typeIds   
    
    const List<word> outputMolecules (propsDict_.lookup("catalysedMolecules"));
    
    if(outputMolecules.size() != inputMolecules.size())
    {
        
        FatalErrorIn("dsmcDiffuseCatalyticWallPatch::setProperties()")
            << "catalysedMolecules must be the same size as "
            << "moleculesToBeCatalysed." << nl << "in: "
            << mesh_.time().system()/"boundariesDict"
            << exit(FatalError);
    }
    
    DynamicList<word> outputMoleculesReduced(0);

    forAll(outputMolecules, i)
    {
        const word& moleculeName(outputMolecules[i]);

        if(findIndex(outputMoleculesReduced, moleculeName) == -1)
        {
            outputMoleculesReduced.append(moleculeName);
        }
    }

    outputMoleculesReduced.shrink();
       
    catalysedTypeIds_.setSize(outputMoleculesReduced.size(), -1);
    
    forAll(outputMoleculesReduced, i)
    {
        const word& moleculeName(outputMoleculesReduced[i]);
        
        label typeId = findIndex(cloud_.typeIdList(), moleculeName);
        
        // check that output molecules belong to the typeIdList
        if(typeId == -1)
        {
            FatalErrorIn("dsmcDiffuseCatalyticWallPatch::setProperties()")
                << "Cannot find type id: " << moleculeName << nl 
                << exit(FatalError);
        }
        
        catalysedTypeIds_[i] = typeId;
    }
    
    const dictionary& heatOfReactionDict
    (
        propsDict_.subDict("heatOfReaction")
    );
    
    heatOfReaction_.clear();
    
    heatOfReaction_.setSize(catalysisTypeIds_.size(), 0.0);
    
    if(heatOfReaction_.size() == inputMolecules.size())
    {
        // set the heat of reactions
       
        forAll(heatOfReaction_, i)
        {
            heatOfReaction_[i] = readScalar
            (
                heatOfReactionDict.lookup(inputMoleculesReduced[i])
            );
        }
    }
    else
    {
        FatalErrorIn("dsmcDiffuseCatalyticWallPatch::setProperties()")
                << "heatOfReaction list must be same size as "
                << "moleculesToBeCatalysed." << nl 
                << exit(FatalError);
    }
    
}

} // End namespace Foam

// ************************************************************************* //
