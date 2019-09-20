{
  char *name    = "SLitBeam01";
  char *listing = "SLitBeam01";
  char *upcom   = "Beam of muons, crystal and APD";
  char *downcom = "Simple example with beam issued from an ellipse";
  gROOT->ProcessLine(".x InitSLitrani.C(5,name,listing,upcom,downcom,kFALSE,kTRUE,kFALSE)");
  //
  // arguments of InitSLitrani:
  //
  // size     size of main canvas of TwoPadDisplay [ 1 <= size <= 8 ]
  // name     name of the produced ROOT file (without ".root")
  // listing  name of the listing file (without ".lis")
  // upcom    name of the comment appearing on top    left of canvas
  // downcom  name of the comment appearing on bottom left of canvas
  // otherseq true if one wants an other sequence of random numbers at each new launching
  // indebug  true if one wants to run Litrani in debug mode
  // WithDate if true, appends date and time to name of root file and name of listing
  //
  //  Simulation of an anisotropic quartz crystal having the form of a TGeoBBox, on top of which
  // is a Phototube(PMT). Between the APD and the
  // crystal, there is a slice of glue, having the shape of a TGeoBBox.
  //  The whole is surrounded by an aluminium coating. This aluminium wrapping has in consequence
  // a shape which is the union of 2 TGeoBBox, slightly bigger than the sum of the crystal and the
  // PMT. The use of union for defining the aluminium wrapping is necessary in order that no overlap
  // occurs and that all 6 faces of the crystal be covered with aluminium, including the part of the
  // upper face not covered by the APD. This is by far simpler than painfully defining thin slices
  // of aluminium for covering each face of the crystal. A small complication arises because we do
  // not want that the wrapping of the APD be made of aluminium, but we require that it be a totally
  // absorbing wrapping, named "totabsorbing". For that, we define a TGeoBBox made of "totabsorbing",
  // slightly bigger than the PMT and containing it and included inside the union shape of aluminium.
  // This is a not too complicate way of describing an equipped crystal, using the possibility offered
  // by the TGeo geometry of placing shapes inside other shapes, and using union in order to avoid
  // extrusion.
  //  Everything is contained in the TGeoBBox "TOP", defining the WCS, world coordinate system. Notice
  // that the second argument of AddNode(), the copy nb, is always 1 in this example, since it never
  // happens in this example that the same volume is inserted twice inside some mother volume.
  //  To describe the beam of muons, which is intended to be issued from an ellipse, we place inside
  // TOP a TGeoEltu, an elliptical tube, named "BEAMCRADLE" at the left of the crystal. We rotate it
  // by 90° around the Ox axis, so that the elliptical face of the TGeoEltu points towards the crystal.
  // We displace the TGeoEltu from run to run along the Oz direction, to study the dependence of the
  // efficiency versus the z position of the ellipse emitting muons.
  //  Notice that the TGeoEltu "BEAMCRADLE" is displaced AFTER the geometry has been closed by a
  // call to gGeoManager->CloseGeometry()! To understand how it is possible [calling
  // TGeoVolume::RemoveNode() and TGeoVolume::AddNode() after having closed the geometry is
  // forbidden], read § "Representing Misalignments of the Ideal Geometry", p340 of the ROOT
  // user manual. Method TLitBeam::MoveCradle() do the trick.
  //
    const char *comTL = "Beam cradle is a TGeoEltu";
    const Color_t  matVacuumColor =  0;
    const Color_t  PbWO4Color     =  9;
    const Color_t  AluColor       =  3;
    const Color_t  GlueColor      = 49;
    const Color_t  TotAbsColor    =  1;
    const Color_t  SiliciumColor  =  2;
    const Color_t  CradleColor    =  5;
    const Double_t IrrA           = 0.0; //Value used for A   when irrelevant !
    const Double_t IrrZ           = 0.0; //Value used for Z   when irrelevant !
    const Double_t IrrRho         = 0.0; //Value used for Rho when irrelevant !
    Int_t mediumindex = 1;
    gCleanOut->fMaxInfo = 1000;
    Short_t ns = -1;
    Bool_t  ok = kFALSE;
    ns = 0;
  //____________________________________________________________________________
  //____________________________________________________________________________
  //
  // Building the geometry
  //____________________________________________________________________________
  //____________________________________________________________________________
  //
    TGeoManager *geom = new TGeoManager("setup","SLitBeam01 of new Litrani");
  //____________________________________________________________________________
  //
  // Materials and media
  //____________________________________________________________________________
  //
  // (1) Vacuum for "TOP"
  //
  //  We define here vacuum as a TGeoMedium, not as a TLitMedium, so that photons cannot
  // travel inside it.
  //
    TGeoMaterial *vacuum_mat = new TGeoMaterial("Vacuum",IrrA,IrrZ,IrrRho);
    TGeoMedium *vacuum = new TGeoMedium("Vacuum",mediumindex++,vacuum_mat);
  //
  // (2) Full pedantic definition of PbWO4!
  //
  //  By this definition, A,Z and density are defined. They are needed by Litrani ONLY if
  // class TLitGamma is used, which is not the case here. We give them here just
  // as an example. Radiation length, interaction length and Moliere radius are also given
  // here, although not used. They would be used only with class TLitCascade. As PbWO4 emits
  // photons when crossed by a particle, we need the emission spectrum of PbWO4, which is
  // "Spectrum_PbWO4La", found in database file SpectraDB.rdb. It was inserted into SpectraDB.rdb
  // using CINT macro Spectrum_PbWO4La.C.
  //
    TGeoElementTable *table = gGeoManager->GetElementTable();
    TGeoElement *Si = table->FindElement("SILICON");
    TGeoElement *O  = table->FindElement("OXYGEN");
    TGeoMixture *quartz_mix = new TGeoMixture("Quartz",3,8.28);
    quartz_mix->AddElement(Si,1);
    quartz_mix->AddElement(O,4);
    TLitMedium *quartz = new TLitMedium("Quartz",mediumindex++,quartz_mix,kFALSE,1.0,"AbsorptionLength_PbWO4");
    quartz->IsIsotropic(1);;
    quartz->FindSpectrum("Spectrum_PbWO4La");
    quartz->dEdx("DEdx_MusInPbWO4__000",100.0);

  // (3) Full pedantic definition of Silicium
    TGeoMaterial *silicium_mat = new TGeoMaterial("Silicium",Si,2.33);
    TLitMedium *silicium = new TLitMedium("Silicium",mediumindex++,silicium_mat,kTRUE,1.0,
     "AbsorptionLength_Silicium");
    silicium->IsIsotropic("RefrIndex_Silicium");
  //  (4) Definition of medium "glue"
    TGeoMaterial *glue_mat = new TGeoMaterial("Glue",IrrA,IrrZ,IrrRho);
    switch (ns) {
    case 1:
      TLitMedium *glue = new TLitMedium("Glue",mediumindex++,glue_mat,kFALSE,1.0,10.0);
      break;
    default:
      TLitMedium *glue = new TLitMedium("Glue",mediumindex++,glue_mat,kFALSE,1.0,
        "AbsorptionLength_Meltmount1704");
      break;
    }
    glue->IsIsotropic("RefrIndex_RTV3145");

  // (5) Definition of media "sodocal" of window of PMs..
    const Double_t La_sodocal = 1000.0; //Absorption length of sodocal
    TGeoMaterial *sodocal_mat = new TGeoMaterial("sodocal",IrrA,IrrZ,IrrRho);
    TLitMedium *sodocal = new TLitMedium("sodocal",mediumindex++,sodocal_mat,kFALSE,1.0,La_sodocal);
    sodocal->IsIsotropic("RefrIndex_Sodocal");
  
  // (7) Definition of media "cathode" photocathode of PMs. As photons do not enter
  //      inside the volume of the PM, but immediatly stop on the photocathode, the
  //      material of the PM does not matter. So we declare it identical with the
  //      material of the entrance window of the PM, sodocal, so that the transition
  //          window of PM ==> photocathode is immediate. However, we are obliged to
  //      give an other name [here "cathode" instead of "sodocal"] to the TLitMedium
  //      of the photocathode, it has to be declared as "sensible", contrary to the
  //      TlitMedium of the window of the PM
  //
    TLitMedium *cathode = new TLitMedium("cathode",mediumindex++,sodocal_mat,kTRUE,1.0,La_sodocal);
    cathode->IsIsotropic("RefrIndex_Sodocal");
  //____________________________________________________________________________
  //
  // Wrappings
  //____________________________________________________________________________

  //  (6) Definition of "TotAbsorbing": a totally absorbing wrapping
    TGeoMaterial *totabs_mat = new TGeoMaterial("TotAbsorbing",IrrA,IrrZ,IrrRho);
    TLitMedium *totabsorbing = new TLitMedium("TotAbsorbing",mediumindex++,totabs_mat,-1.0);
    totabsorbing->SetAsWrapping(0.0,0.0,1.0,1.0,1.0);
    totabsorbing->IsIsotropic(1.0);
  //  (7) Definition of wrapping medium aluminium
    const Double_t Air_RefrIndex     = 1.0003;   //Refractive index of air
    const Double_t aluminium_diffus  = 0.01;
    const Double_t AluminiumSupplAbs = 0.1;
    TGeoMaterial *aluminium_mat = new TGeoMaterial("Aluminium",IrrA,IrrZ,IrrRho);
    TLitMedium *aluminium = new TLitMedium("Aluminium",mediumindex++,aluminium_mat,1.0);
    aluminium->SetAsWrapping(aluminium_diffus,"RIndexRev_Aluminium","IIndexRev_Aluminium",1.0,AluminiumSupplAbs);
    aluminium->IsIsotropic(Air_RefrIndex);

  // (9) Definition of "WrapPM" : a strongly absorbing wrapping around the PMs
    const Double_t wrapPM_diffus = 0.9;
    const Double_t wrapPM_Rindex = 2.5;
    const Double_t wrapPM_Iindex = 1.5;
    const Double_t wrapPM_suppl  = 0.9;
    TGeoMaterial *wrapPM_mat = new TGeoMaterial("WrapPM",IrrA,IrrZ,IrrRho);
    TLitMedium *wrappm = new TLitMedium("WrapPM",mediumindex++,wrapPM_mat,-1.0);
    wrappm->SetAsWrapping(wrapPM_diffus,wrapPM_Rindex,wrapPM_Iindex,1.0,wrapPM_suppl);
    wrappm->IsIsotropic(1.0);  // Only useful if last arg of constructor changed to +1.0

  //____________________________________________________________________________
  //
  // Dimensions
  //____________________________________________________________________________
  // Dimmensions of Photodetectors
  //
    const Double_t PMrev_r  = 5.6;
    const Double_t PMrev_dz = 1.5;
    const Double_t PMrev_w  = 0.2;  //width of wrapping around PM
    const Double_t PMwin_r  = PMrev_r - PMrev_w;
    const Double_t PMwin_dz = 4.8;  //thickness of window of PM is 0.4
    const Double_t PM_r     = PMwin_r;
    const Double_t PM_dz = (2*PMrev_dz - PMrev_w - 2*PMwin_dz)/2.0;
  // Dimensions of crystal - 5.6cm x 5.6cm x 1cm
  //
    const Double_t crys_dx = 5.6;
    const Double_t crys_dy = 5.6;
    const Double_t crys_dz = 1.0;
  //
  // Dimensions of beam cradle of type TGeoEltu inside TOP
  //
    const Double_t crad_dx = 0.6;
    const Double_t crad_dy = 0.3;
    const Double_t crad_dz = 0.6;
  //
  // Start position of beam cradle and step size
  //
    const Double_t frsp   = -2.0; //frsp == free space
    const Double_t stepz  = 1.0;
    const Double_t starty = 0.0;
    Double_t startz = -crys_dz + frsp;
    Double_t arun  = 2*(crys_dz-frsp)/stepz + 1.0;
    Int_t nrun = (Int_t)arun;
  //
  // Dimensions of PMT - 4.8cm x 4.8cm x 1.5cm
  //
    const Double_t apd_dx = 4.8;
    const Double_t apd_dy = 4.8;
    const Double_t apd_dz = 1.5;
  //
  // Dimensions for glue of APD
  //
    const Double_t glueapd_dx = apd_dx;
    const Double_t glueapd_dy = apd_dy;
    const Double_t glueapd_dz[2] = {0.05,0.5};
  //
  // Width of wrapping is wid
  //
    const Double_t wid[2] = {0.1,0.5};
    Double_t ws2 = 0.5*wid[ns];
  //
  // Dimensions of TGEOBBox of alu around crystal "CRYSTALWS"
  //
    Double_t aluc_dx = crys_dx + wid[ns];
    Double_t aluc_dy = crys_dy + wid[ns];
    Double_t aluc_dz = crys_dz + wid[ns];
  //
  // Dimensions of TGEOBBox of alu around APD and glue of APD [partially] "APDWS"
  //
    Double_t alua_dx = apd_dx + wid[ns];
    Double_t alua_dy = apd_dy + wid[ns];
    Double_t alua_dz = apd_dz + glueapd_dz[ns];
  //
  // Dimensions of TotAbsorbing wrapping around APD and glue of APD
  //
    Double_t tot_dx = apd_dx + wid[ns];
    Double_t tot_dy = apd_dy + wid[ns];
    Double_t tot_dz = apd_dz + glueapd_dz[ns] + ws2;
  //
  // Dimensions of TOP
  //
    const Double_t top_dx    = 20.0;
    const Double_t top_dy    = 20.0;
    Double_t top_dz = wid[ns] + crys_dz + glueapd_dz[ns] + apd_dz + 1.0;
  //____________________________________________________________________________
  //
  // Positioning (translations)
  //____________________________________________________________________________
  //
  // (1) Positioning of PM inside PMrev
  //
    const Double_t t_PM_z = PMrev_dz - PMrev_w - PM_dz;
    TGeoTranslation *t1 = new TGeoTranslation("t1",0.0,0.0,t_PM_z);
  //
  // (2) Positionning of PMwin inside PMrev
  //
    const Double_t t_PMwin_z = PMwin_dz - PMrev_dz;
    TGeoTranslation *t2 = new TGeoTranslation("t2",0.0,0.0,t_PMwin_z);
  //  t1 is for positionning the TGeoBBox of aluminium around the TGeoBBox of "totabsorbing" [itself
  // around TGeoBBox APD and TGeoBBox of glue of APD] with respect to the TGeoBBox of aluminium around
  // crystal [or equivalently, with respect to the composite shape "rev_shape"].
  //  t1 is used for the creation of composite shape, so it has to be named and registered by
  // a call to RegisterYourself().
  //
    const Double_t t_alua_crys_z = aluc_dz + alua_dz;
    TGeoTranslation *t1 = new TGeoTranslation("t1",0.0,0.0,t_alua_crys_z);
    t1->RegisterYourself();
  //  t3 is for positionning the TGeoBBox of "totabsorbing" around APD and around glue of APD
  // with respect to the TGeoBBox of aluminium around crystal [or equivalently, with respect
  // to the composite shape "rev_shape"].
  //
    const Double_t t_tot_crys_z = crys_dz + alua_dz + ws2;
    TGeoTranslation *t3 = new TGeoTranslation("t3",0.0,0.0,t_tot_crys_z);
  //  t4 is for positionning the TGeoBBox of APD inside the TGeoBBox of "totabsorbing"
  //  around APD and glue of APD "TOT"
  //
    Double_t t_apd_tot_z = glueapd_dz[ns] - ws2;
    TGeoTranslation *t4 = new TGeoTranslation("t4",0.0,0.0,t_apd_tot_z);
  //  t5 is for positionning TGeoBBox of glue of the APD inside the TGeoBBox of "totabsorbing"
  //  around APD and glue of APD "TOT"
  //
    const Double_t t_glueapd_tot_z = -(apd_dz + ws2);
    TGeoTranslation *t5 = new TGeoTranslation("t5",0.0,0.0,t_glueapd_tot_z);
  //  ph6 is for the start position of the beam cradle inside TOP
    Double_t phi,theta,psi;
    TGeoRotation r6;
    TEuler euler(90.0,0.0,90.0,90.0);
    euler.GetEulerAngles(phi,theta,psi);
    r6.SetAngles(phi,theta,psi);
    TGeoTranslation t6(0.0,starty,startz);
    TGeoCombiTrans *c6 = new TGeoCombiTrans(t6,r6);
    TGeoHMatrix *ph6 = new TGeoHMatrix(*c6);
  //____________________________________________________________________________
  //
  // Volumes and nodes
  //____________________________________________________________________________

  // Top box containing everything and defining the WCS, world coordinate system
  
    TGeoVolume *top = geom->MakeBox("TOP",vacuum,top_dx,top_dy,top_dz);
    geom->SetTopVolume(top);
  
  // Composite shape aluminium
    TGeoBBox *revcrys_shape = new TGeoBBox("CRYSTALWS",aluc_dx,aluc_dy,aluc_dz);
    TGeoBBox *revapd_shape  = new TGeoBBox("APDWS",alua_dx,alua_dy,alua_dz);
    TGeoCompositeShape *rev_shape = new TGeoCompositeShape("REV","CRYSTALWS+APDWS:t1");
    TGeoVolume *rev = new TGeoVolume("REV",rev_shape,aluminium);
    top->AddNode(rev,1);

  // TGeoBBox of "totabsorbing" around APD and glue of APD
  
    TGeoVolume *tot = geom->MakeBox("TOT",totabsorbing,tot_dx,tot_dy,tot_dz);
    rev->AddNode(tot,1,t3);
  // (7) TGeoTube PMrev is the wrapping surrounding all the PM [except the window
  //                    of the PM]. It is the box containing the phototube

    TGeoVolume *PMrev = geom->MakeTube("PMREV",wrappm,0.0,PMrev_r,PMrev_dz);

  // (8) TGeoTube PMwin is the sodocal window of the PM

    TGeoVolume *PMwin = geom->MakeTube("PMWIN",sodocal,0.0,PMwin_r,PMwin_dz);
    PMrev->AddNode(PMwin,1,t2);

  // (9) TGeoTube PM is the PM itself

    TGeoVolume *PM = geom->MakeTube("PM",cathode,0.0,PM_r,PM_dz);
    PMrev->AddNode(PM,1,t1);
    TLitVolume *Lit_PM = new TLitVolume(PM);
    Lit_PM->SetPhotoTube("QuantumEff_XP2020",1.0);
    tot->AddNode(PM,1,t4);

  // TGeoBBox of glue of APD

    TGeoVolume *glueapd = geom->MakeBox("GlueAPD",glue,glueapd_dx,glueapd_dy,glueapd_dz[ns]);
    tot->AddNode(glueapd,1,t5);
  
  // TGeoBBox crystal made of PbWO4 is centered inside "rev"
  
    TGeoVolume *crystal = geom->MakeBox("CRYSTAL",quartz,crys_dx,crys_dy,crys_dz);
    rev->AddNode(crystal,1);
  
  // TGeoEltu beam cradle made of vacuum inside TOP.
  
    TGeoVolume *cradle = geom->MakeEltu("BEAMCRADLE",vacuum,crad_dx,crad_dy,crad_dz);
    top->AddNode(cradle,1,ph6);
  
    geom->CloseGeometry();
    geom->CheckOverlaps(0.01);
  //____________________________________________________________________________
  //____________________________________________________________________________
  //
  // End of Building the geometry
  //____________________________________________________________________________
  //____________________________________________________________________________
  //
  //____________________________________________________________________________
  //
  // Colors and drawing
  //____________________________________________________________________________
  //
    top->SetVisibility(kFALSE);
    top->SetVisContainers();
    top->SetLineColor(1);
    top->SetLineWidth(1);
    rev->SetLineColor(AluColor);
    rev->SetLineWidth(1);
    PM->SetVisibility(kTRUE);
    PM->SetLineColor(SiliciumColor);
    PM->SetLineWidth(1);
    crystal->SetVisibility(kTRUE);
    crystal->SetLineColor(PbWO4Color);
    crystal->SetLineWidth(1);
    cradle->SetVisibility(kTRUE);
    cradle->SetLineColor(CradleColor);
    cradle->SetLineWidth(1);
    gGeoManager->SetTopVisible(1);
    gGeoManager->SetVisLevel(4);
    TLit::Get()->BookCanvas(5);
    gTwoPad->SetStateOfPads(Pad1Only);
    gTwoPad->SetAllGrey();
    gTwoPad->CommentTL(comTL);
    top->Draw("");
  //____________________________________________________________________________
  //____________________________________________________________________________
  //
  // Generation of photons
  //____________________________________________________________________________
  //____________________________________________________________________________

    Int_t krun;
    TLitParticle *particle = new TLitParticle(13,gaussian,100.0,90.0,110.0,5.0);
    TVector3 dir(0.0,0.0,1.0); //0,0,1
    TVector3 dirfce(0.0,0.0,1.0);
    TVector3 zero_vector(0.0,0.0,0.0);
    TLitBeam *beam = new TLitBeam("muons","beam of muons","/TOP_1/BEAMCRADLE_1",particle,kTRUE,kFALSE);
    beam->SetEmission(flat,5.0,dir,"",kFALSE,zero_vector,kTRUE,dirfce);


    Double_t xyz[3];
    xyz[0] = 0.0;
    xyz[1] = starty;
    //for (krun=1;krun<=nrun;krun++) {
      beam->Gen(krun,1,startz);
     //if (krun != nrun) {
     //}
   //}
    
    gTwoPad->SetStateOfPads(Pad1AndPad2);
    gTwoPad->ChangePad();
    gLitGs->DoStat();
    TLit::Get()->CloseFiles();
}
