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

Class
    dissociationIonisationTypeIDissimilarSpecies

Description


SourceFiles
    dissociationIonisationTypeIDissimilarSpecies.C


\*---------------------------------------------------------------------------*/

#ifndef dissociationIonisationTypeIDissimilarSpecies_H
#define dissociationIonisationTypeIDissimilarSpecies_H

#include "dsmcReaction.H"
#include "vector.H"
#include "fvMesh.H"
#include "volFields.H"
#include "dsmcCloud.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class dissociationIonisationTypeIDissimilarSpecies Declaration
\*---------------------------------------------------------------------------*/

class dissociationIonisationTypeIDissimilarSpecies
:
   public dsmcReaction
{

private:
    // Private data

        //- subDictionary containing the properties
        dictionary propsDict_;

        List<label> reactantIds_;

        List<List <label> > dissociationProducts_;
        List<List <label> > ionisationProducts_;
        
        word reactionName_;

        scalar heatOfReactionDissociationAB_;
        scalar heatOfReactionIonisationAB_;
        scalar heatOfReactionDissociationCD_;
        scalar heatOfReactionIonisationCD_;
        
        label nTotABDissociationReactions_;
        label nABDissociationReactionsPerTimeStep_;
        label nTotABIonisationReactions_;
        label nABIonisationReactionsPerTimeStep_;
        label nTotCDDissociationReactions_;
        label nCDDissociationReactionsPerTimeStep_;
        label nTotCDIonisationReactions_;
        label nCDIonisationReactionsPerTimeStep_;

        bool relax_;

        // remove if not used for heat bath
        bool allowSplitting_;
        bool writeRatesToTerminal_;
        scalar volume_;
        List<scalar> numberDensities_;


    // Private Member Functions

        void setProperties();


public:

    //- Runtime type information
    TypeName("dissociationIonisationTypeIDissimilarSpecies");

    // Constructors

        //- Construct from components
        dissociationIonisationTypeIDissimilarSpecies
        (
	        Time& t,
            dsmcCloud& cloud,
            const dictionary& dict
        );
    
        // Destructor
        ~dissociationIonisationTypeIDissimilarSpecies();
    
    // Member Functions

        // initial configuration - read in properties from chemReactDict
        void initialConfiguration();

        // apply a reaction between parcel p and q
        void reaction
        (
            dsmcParcel& p,
            dsmcParcel& q,
            const DynamicList<label>& candidateList,
            const List<DynamicList<label> >& candidateSubList,
            const label& candidateP,
            const List<label>& whichSubCell
        );

        void reaction
        (
            dsmcParcel& p,
            dsmcParcel& q
        );

        // return bool if molecules belong to this reaction model
        bool tryReactMolecules(const label& typeIdP, const label& typeIdQ);

        // Access

        // return bool for relax = true (run normal collision model)
        // relax = false (apply reaction to molecules)
        const bool& relax() const;

        void outputResults(const label& counterIndex);

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
