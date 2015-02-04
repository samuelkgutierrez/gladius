/*
 * Copyright (c) 2014-2015 Los Alamos National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the Gladius project. See the LICENSE.txt file at the
 * top-level directory of this distribution.
 *
 * Copyright (c) 2008-2012, Lawrence Livermore National Security, LLC
 */

#include "tool-be.h"

#include "core/core.h"
#include "core/colors.h"
#include "tool-common/tool-common.h"

#include "lmon_api/lmon_be.h"

#include <cstdlib>
#include <string>

using namespace gladius;
using namespace gladius::toolbe;

////////////////////////////////////////////////////////////////////////////////
// ToolBE
////////////////////////////////////////////////////////////////////////////////
namespace {
// This component's name.
const std::string CNAME = "toolbe";
// CNAME's color code.
const std::string NAMEC =
    core::colors::color().ansiBeginColor(core::colors::NONE);
// Convenience macro to decorate this component's output.
#define COMP_COUT GLADIUS_COMP_COUT(CNAME, NAMEC)
// Output if this component is being verbose.
#define VCOMP_COUT(streamInsertions)                                           \
do {                                                                           \
    if (this->mBeVerbose) {                                                    \
        COMP_COUT << streamInsertions;                                         \
    }                                                                          \
} while (0)

/**
 *
 */
int
feToBeUnpack(
    void *buf,
    int bufLen,
    void *data
) {
    GLADIUS_UNUSED(buf);
    GLADIUS_UNUSED(bufLen);
    GLADIUS_UNUSED(data);
    return 0;
}
}

/**
 * Constructor.
 */
ToolBE::ToolBE(
    void
) : mBeVerbose(false)
{
}

/**
 * Destructor.
 */
ToolBE::~ToolBE(void)
{
}

/**
 *
 */
void
ToolBE::mInitLMON(
    const core::Args &args,
    bool beVerbose
) {
    mLMONBE.init(args, beVerbose);
    // We know how to do this, so let LMON know what to call.
    mLMONBE.regUnpackForFEToBE(feToBeUnpack);
    mLMONBE.handshake();
    // Let LMON populate our process table.
    mLMONBE.createAndPopulateProcTab(mProcTab);
}

/**
 *
 */
void
ToolBE::init(
    const core::Args &args,
    bool beVerbose
) {
    try {
        mBeVerbose = beVerbose;
        mArgs = args;
        VCOMP_COUT("Initializing Tool Back-End..." << std::endl);
        //
        mInitLMON(mArgs, mBeVerbose);
        //
        mMRNBE.init(mBeVerbose);
    }
    catch (const std::exception &e) {
        throw core::GladiusException(GLADIUS_WHERE, e.what());
    }
}

/**
 * TODO FIXME
 */
void
ToolBE::redirectOutputTo(
    const std::string &base
) {
    GLADIUS_UNUSED(base);
    std::string fName = "/tmp/BE-" + std::to_string(getpid()) + ".txt";
    FILE *outRedirectFile = freopen(fName.c_str(), "w", stdout);
    if (!outRedirectFile) GLADIUS_THROW_CALL_FAILED("freopen");
}

/**
 *
 */
void
ToolBE::connect(void)
{
}

/**
 *
 */
void
ToolBE::mainLoop(void)
{
    VCOMP_COUT("Entering Main Loop" << std::endl);
}

/**
 *
 */
void
ToolBE::finalize(void)
{
    mLMONBE.finalize();
}
