clc
close all
clear all
format long

%% Constantes et données connus
TORQUE_PETIT_MOTEUR_MAX = 3;%Nm
TORQUE_GROS_MOTEUR_MAX = 3;%Nm
mPload = 0.15;%kg

mC = 16*10^-3;%kg
g = 9.81; %m/s^2

Lccm = 24*10^-3;%m À CHANGER PAS BON JAI CHANGER LA PIECE SANS LE DIRE COMME UN VOLEUR
Lpload = 39.94*10^-3;%m À CHANGER PAS BON JAI CHANGER LA PIECE SANS LE DIRE COMME UN VOLEUR

thetaA = pi/2; %rad
thetaB = pi/2; %rad
thetaC = pi/2; %rad

%% Torque poignet
Fgc = g*mC;
Fg_pload = g*mPload;

Tm_poignet = sin(thetaC)*(Lpload*Fg_pload + Lccm*Fgc)

mass_total_poignet = Fgc+Fg_pload;

L_vs_deux_m = Lpload-Lccm;
cm_poignet = (0*mC+mPload*L_vs_deux_m)/(mass_total_poignet/g);
L_cm_poignet = cm_poignet + Lccm;

Tm_poignet_CM = sin(thetaC)*(L_cm_poignet*mass_total_poignet)
%% Torque Coude
L_coude2 = 65*10^-3;%m
L_coude2cm_ = 27*10^-3;%m
L_dBeam = 4.23*10^-3;%m
L_Bbeam_ = L_coude2+(L_dBeam/2);%m
L_poignet1cm = 40*10^-3;%m
L_poignet1Moteur = 58*10^-3;%m
L_c1m_ = L_coude2+L_dBeam+L_poignet1cm;%m
L_cTm_p_ = L_coude2+L_dBeam+L_poignet1Moteur;%m
% A VÉRIFIER
Lv1_ = 38*10^-3;%m
Lv2_ = 58*10^-3;%m
Lv3_ = 76.23*10^-3;%m
Lv4_ = 91.23*10^-3;%m
Lshaft_ = 135.85*10^-3;%m
L_vis_moteur_ = 122.58*10^-3;%m À VERIFIER J'AI MIS DISTANCE MILLIEUX ENTRE LES DEUX TROUES OBLONG

LB_ = L_cTm_p_+12*10^-3;%m

m_coude2 = 19*10^-3;%kg
mBeam = 10*10^-3;%kg
m_poignet1 = 32*10^-3;%kg
m_moteurp = 57*10^-3;%kg
m_vis = 4*10^-3;%kg
m_vis_moteur = 4*0.5*10^-3;%kg
m_shaft = 0;%kg NÉGLIGEABLE

Fg_coude2 = m_coude2*g;
Fg_Beam = mBeam*g;
Fg_poignet1 = m_poignet1*g;
Fg_moteurp = m_moteurp*g;
Fg_vis_moteur = m_vis_moteur*g;

Fg_vis = m_vis*g;
Fg_shaft = m_shaft*g;

T_mass_coude = L_coude2cm_*Fg_coude2 + L_Bbeam_*Fg_Beam + L_c1m_*Fg_poignet1 + L_cTm_p_*Fg_moteurp + Fg_vis*(Lv1_+Lv2_+Lv3_+Lv4_) + Fg_vis_moteur*L_vis_moteur_ + Fg_shaft*Lshaft_;
Tm_coude = Tm_poignet + sin(thetaB)*(LB_*(Fgc+Fg_pload)+T_mass_coude)

masse_totale_coude = m_coude2 + mBeam + m_poignet1 + m_moteurp + m_vis*4 + m_vis_moteur + m_shaft;
Fg_totale_coude = masse_totale_coude*g;

%Distance des différents centre de masse par rapport au centre de masse "L_coude2cm_"
L_coude2cm_R = L_coude2cm_ - L_coude2cm_;
L_Bbeam_R = L_Bbeam_ - L_coude2cm_;
L_c1m_R = L_c1m_ - L_coude2cm_;
L_cTm_p_R = L_cTm_p_- L_coude2cm_;
Lv1_R = Lv1_ - L_coude2cm_;
Lv2_R = Lv2_ - L_coude2cm_;
Lv3_R = Lv3_ - L_coude2cm_;
Lv4_R = Lv4_ - L_coude2cm_;
Lshaft_R = Lshaft_ - L_coude2cm_;
L_vis_moteur_R = L_vis_moteur_ - L_coude2cm_;

cm_coude = (m_coude2*L_coude2cm_R + mBeam*L_Bbeam_R + m_poignet1*L_c1m_R + m_moteurp*L_cTm_p_R + m_vis*(Lv1_R + Lv2_R + Lv3_R + Lv4_R) + m_vis_moteur*L_vis_moteur_R + m_shaft*Lshaft_R)/(masse_totale_coude);
L_cm_coude = cm_coude + L_coude2cm_;%À PARTIR DU CENTRE DES TROUES MOTEUR (OÙ LES MOTEURS APPLIQUE LEURS TORQUES EN GROS)
Tm_coude_CM = Tm_poignet + sin(thetaB)*(LB_*(Fgc+Fg_pload)+(L_cm_coude*Fg_totale_coude))

%% Torque Épaule

%VÉRIFIER TOUTE LES MESURE J'AI COPIER CEUX DE COUDE PUISQUE DEVERAIS ÊTRE
%LA MÊME CHOSE MAIS À VÉRIFIER

L_epaule2 = 65*10^-3;%m
L_epaule2cm_ = 27*10^-3;%m

L_coude1cm = 40*10^-3;%m
L_coude1Moteur = 58*10^-3;%m
L_b1m_ = L_epaule2+L_dBeam+L_coude1cm;%m
L_bTm_p_ = L_epaule2+L_dBeam+L_coude1Moteur;%m

LA_ = L_cTm_p_+12*10^-3;%m

m_epaule2 = 19*10^-3;%kg
m_coude1 = 34*10^-3;%kg

Fg_epaule2 = m_epaule2*g;
Fg_coude1 = m_coude1*g;

T_mass_epaule = L_epaule2cm_*Fg_epaule2 + L_Bbeam_*Fg_Beam + L_c1m_*Fg_coude1 + L_cTm_p_*Fg_moteurp + Fg_vis*(Lv1_+Lv2_+Lv3_+Lv4_) + Fg_vis_moteur*L_vis_moteur_ + Fg_shaft*Lshaft_;
Tm_epaule = Tm_coude + sin(thetaA)*(LA_*(Fg_totale_coude+Fgc+Fg_pload)+T_mass_epaule)

masse_totale_epaule = m_epaule2 + mBeam + m_coude1 + m_moteurp + m_vis*4 + m_vis_moteur + m_shaft;
Fg_totale_epaule = masse_totale_epaule*g;

%Distance des différents centre de masse par rapport au centre de masse "L_epaule2cm_"
L_epaule2cm_R = L_epaule2cm_ - L_epaule2cm_;
L_Bbeam_R = L_Bbeam_ - L_epaule2cm_;
L_c1m_R = L_c1m_ - L_epaule2cm_;
L_cTm_p_R = L_cTm_p_- L_epaule2cm_;
Lv1_R = Lv1_ - L_epaule2cm_;
Lv2_R = Lv2_ - L_epaule2cm_;
Lv3_R = Lv3_ - L_epaule2cm_;
Lv4_R = Lv4_ - L_epaule2cm_;
Lshaft_R = Lshaft_ - L_epaule2cm_;
L_vis_moteur_R = L_vis_moteur_ - L_epaule2cm_;

cm_epaule = (m_epaule2*L_epaule2cm_R + mBeam*L_Bbeam_R + m_coude1*L_c1m_R + m_moteurp*L_cTm_p_R + m_vis*(Lv1_R + Lv2_R + Lv3_R + Lv4_R) + m_vis_moteur*L_vis_moteur_R + m_shaft*Lshaft_R)/(masse_totale_epaule);
L_cm_epaule = cm_epaule + L_epaule2cm_;%À PARTIR DU CENTRE DES TROUES MOTEUR (OÙ LES MOTEURS APPLIQUE LEURS TORQUES EN GROS)
Tm_epaule_CM = Tm_coude + sin(thetaA)*(LA_*(Fg_totale_coude+Fgc+Fg_pload)+(L_cm_epaule*Fg_totale_epaule))



