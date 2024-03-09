clc
close all
clear all
format long

%% Constantes
mPload = 0.2;%kg

m_coude2 = 19*10^-3;%kg
mBeam = 20*10^-3;%kg A VERIFIER PAS BON
m_poignet1 = 37*10^-3;%kg
m_moteurp = 57*10^-3;%kg

mC = 18*10^-3;%kg
g = 9.81; %m/s^2

L_coude2 = 65*10^-3;%m
L_coude2cm_ = 27*10^-3;%m
L_dBeam = 0*10^-3;%m
L_Bbeam_ = L_coude2+(L_dBeam/2);%m
L_poignet1cm = 40*10^-3;%m
L_poignet1Moteur = 58*10^-3;%m
L_c1m_ = L_coude2+L_dBeam+L_poignet1cm;%m
L_cTm_p_ = L_coude2+L_dBeam+L_poignet1Moteur;%m
LB_ = L_cTm_p_+12*10^-3;%m

Lccm = 24*10^-3;%m À CHANGER PAS BON JAI CHANGER LA PIECE SANS LE DIRE COMME UN VOLEUR
Lpload = 39.94*10^-3;%m À CHANGER PAS BON JAI CHANGER LA PIECE SANS LE DIRE COMME UN VOLEUR

thetaA = pi/2; %rad
thetaB = pi/2; %rad
thetaC = pi/2; %rad

%% Torque poignet
Fgc = g*mC;
Fg_pload = g*mPload;

Tm_poignet = sin(thetaC)*(Lpload*Fg_pload + Lccm*Fgc)

%% Torque Coude
Fg_coude2 = m_coude2*g;
Fg_Beam = mBeam*g;
Fg_poignet1 = m_poignet1*g;
Fg_moteurp = m_moteurp*g;

T_mass_coude = L_coude2cm_*Fg_coude2 + L_Bbeam_*Fg_Beam + L_c1m_*Fg_poignet1 + L_cTm_p_*Fg_moteurp
Tm_coude = Tm_poignet + sin(thetaB)*(LB_*(Fgc+Fg_pload)+T_mass_coude)






