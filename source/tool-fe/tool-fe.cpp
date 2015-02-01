/*
 * Copyright (c) 2014-2015 Los Alamos National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the Gladius project. See the LICENSE.txt file at the
 * top-level directory of this distribution.
 */

#include "tool-fe.h"
#include "core/utils.h"
#include "tool-be/tool-be.h"

#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * To fully understand the entire set of interactions here, you'll also need to
 * see the back-end versions of the tool, LaunchMon, and MRNet.
 */

using namespace gladius;
using namespace gladius::toolfe;

namespace {
// This component's name.
static const std::string CNAME = "tool-fe";
// CNAME's color code.
static const std::string NAMEC =
    core::colors::color().ansiBeginColor(core::colors::GREEN);
// Convenience macro to decorate this component's output.
#define COMP_COUT GLADIUS_COMP_COUT(CNAME, NAMEC)

/**
 *
 */
void
echoLaunchStart(const gladius::core::Args &args)
{
    std::string lstr;
    for (decltype(args.argc()) i = 0; i < args.argc(); ++i) {
        lstr += args.argv()[i];
        lstr += " ";
    }
    GLADIUS_COUT_STAT << "Launch Sequence Initiated..." << std::endl;
    GLADIUS_COUT_STAT << "Starting: " << lstr << std::endl;
}

} // end namespace

/**
 *
 */
void
ToolFE::mGetStateFromEnvs(void)
{
    if (core::utils::envVarSet(GLADIUS_TOOL_FE_VERBOSE_STR)) {
        mBeVerbose = true;
    }
    else {
        mBeVerbose = false;
    }
}

/**
 * Tool front-end constructor.
 */
ToolFE::ToolFE(
    void
) : mBeVerbose(false)
  , mConnectionTimeoutInSec(toolcommon::unlimitedTimeout)
  , mMaxRetries(toolcommon::unlimitedRetries)
{
    mGetStateFromEnvs();
}

/**
 * Returns whether or not the tool-fe's environment setup is sane.
 */
bool
ToolFE::envSane(std::string &whatsWrong)
{
    GLADIUS_UNUSED(whatsWrong);
    bool sane = true;
    return sane;
}

/**
 * Responsible for running the tool front-end instance. This is the tool-fe
 * entry point from a caller's perspective.
 */
void
ToolFE::mainLoop(
    const core::Args &args
) {
    if (mBeVerbose) {
        COMP_COUT << "Entering Main Loop." << std::endl;
    }
    try {
        mAppArgs = args;
        // Make sure that all the required bits are
        // set before we get to launching anything.
        std::string whatsWrong;
        if (!envSane(whatsWrong)) {
            GLADIUS_CERR << whatsWrong << std::endl;
            return;
        }
        // If we are here, then our environment is sane enough to start...
        // FIXME dup stdout?
        mInitializeToolInfrastructure();
        // Start lash-up thread.
        mStartToolLashUpThread();
    }
    // If something went south, just print the haps and return to the top-level
    // REPL. Insulate the caller by catching things and handling them here.
    catch (const std::exception &e) {
        GLADIUS_CERR << e.what() << std::endl;
    }
}

/**
 *
 */
void
ToolFE::mInitializeToolInfrastructure(void)
{
    try {
        // First init LaunchMON
        mLMONFE.init(mBeVerbose);
        // Register function that is responsible for packing data for front-end
        // to back-end transfers. The MRNetFE know how to do this.
        mLMONFE.regPackForFeToBe(mrnet::MRNetFE::getFEToBePackFun());
        // Then do the same for MRNet
        mMRNFE.init(mBeVerbose);
    }
    catch (const std::exception &e) {
        throw core::GladiusException(GLADIUS_WHERE, e.what());
    }
}

/**
 *
 */
void
ToolFE::mStartToolLashUpThread(void)
{
    std::unique_lock<std::mutex> lock(mtLashUpLock);
    std::thread luThread(&ToolFE::mInitiateToolLashUp, this);
    mtLashUpComplete.wait(lock);
    luThread.join();
    if (GLADIUS_SUCCESS != maStatus) {
        mLMONFE.shutdown();
        GLADIUS_THROW_CALL_FAILED_RC("mStartToolLashUpThread", maStatus);
    }
}

/**
 * The thread that initiates the tool lash-up.  This is NOT the main thread, so
 * this is why we don't throw in the exceptional case.
 */
void
ToolFE::mInitiateToolLashUp(void)
{
    try {
        maStatus = GLADIUS_SUCCESS;
        echoLaunchStart(mAppArgs);
        // And so it begins...
        mLMONFE.launchAndSpawnDaemons(mAppArgs);
        // Make sure that the tool daemons launched.
        if (!mLMONFE.daemonsLaunched()) {
            GLADIUS_THROW("Tool Daemons Not Launched.");
        }
        // Create MRNet network FE.
        mMRNFE.createNetworkFE(mLMONFE.getProcTab());
        // Send info to daemons.
        mLMONFE.sendDaemonInfo(mMRNFE.getLeafInfo());
        // Connect the MRNet tree.
        // SKG Resume
        mMRNFE.connect();
    }
    catch (const std::exception &e) {
        GLADIUS_CERR << e.what() << std::endl;
        maStatus = GLADIUS_ERR_LMON;
    }
    // Notify main thread unconditionally.
    mtLashUpComplete.notify_one();
}
