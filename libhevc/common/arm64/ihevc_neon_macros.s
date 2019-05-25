///*****************************************************************************
//*
//* Copyright (C) 2012 Ittiam Systems Pvt Ltd, Bangalore
//*
//* Licensed under the Apache License, Version 2.0 (the "License");
//* you may not use this file except in compliance with the License.
//* You may obtain a copy of the License at:
//*
//* http://www.apache.org/licenses/LICENSE-2.0
//*
//* Unless required by applicable law or agreed to in writing, software
//* distributed under the License is distributed on an "AS IS" BASIS,
//* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//* See the License for the specific language governing permissions and
//* limitations under the License.
//*
//*****************************************************************************/
///**
//*******************************************************************************
//* @file
//*  ihevc_neon_macros.s
//*
//* @brief
//*  Contains assembly macros
//*
//* @author
//*  Naveen SR
//*
//* @par List of Functions:
//*
//*
//* @remarks
//*  None
//*
//*******************************************************************************


.macro push_v_regs
    stp         d8,d9,[sp,#-16]!
    stp         d10,d11,[sp,#-16]!
    stp         d12,d13,[sp,#-16]!
    stp         d14,d15,[sp,#-16]!
.endm
.macro pop_v_regs
    ldp         d14,d15,[sp],#16
    ldp         d12,d13,[sp],#16
    ldp         d10,d11,[sp],#16
    ldp         d8,d9,[sp],#16
.endm
