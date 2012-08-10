#include <IOKit/IOLib.h>
#include <IOKit/IOTimerEventSource.h>
#include "MyDriver.h"

// This required macro defines the class's constructors, destructors,
// and several other methods I/O Kit requires.
OSDefineMetaClassAndStructors(com_JMC_driver_MyDriver, IOEthernetController)

// Define the driver's superclass.
#define super IOEthernetController

bool com_JMC_driver_MyDriver::init(OSDictionary *dict)
{
    bool result = super::init(dict);
    IOLog("Initializing\n");
    return result;
}

void com_JMC_driver_MyDriver::free(void)
{
    IOLog("Freeing\n");
    super::free();
}

IOService *com_JMC_driver_MyDriver::probe(IOService *provider,
                                                SInt32 *score)
{
    IOService *result = super::probe(provider, score);
    IOLog("Probing\n");
    return result;
}

/*
bool com_JMC_driver_MyDriver::start(IOService *provider)
{
    bool result = super::start(provider);
    IOLog("Starting\n");
    return result;
}
*/
bool com_JMC_driver_MyDriver::start(IOService* provider)
{
    if (!super::start(provider))
        return false;
    if (!createMediumDict())
        return false;
    fWorkLoop = getWorkLoop();
    if (!fWorkLoop)
        return false;
    fWorkLoop->retain();
    if (attachInterface((IONetworkInterface**)&fNetworkInterface) == false)
        return false;
    fNetworkInterface->registerService();
    /*fInterruptSource = IOTimerEventSource::timerEventSource(this, interruptOccured);
    if (!fInterruptSource)
        return false;
    if (fWorkLoop->addEventSource(fInterruptSource) != kIOReturnSuccess)
        return false;*/
    IOLog("%s::start() -> success\n", getName());
    return true;
}

void com_JMC_driver_MyDriver::stop(IOService *provider)
{
    IOLog("Stopping\n");
    super::stop(provider);
}

bool com_JMC_driver_MyDriver::configureInterface(IONetworkInterface *netif)
{
    IONetworkData* nd;
    if (super::configureInterface(netif) == false)
        return false;
    nd = netif->getNetworkData(kIONetworkStatsKey);
    if (!nd || !(fNetworkStats = (IONetworkStats *)nd->getBuffer()))
        return false;
    nd = netif->getParameter(kIOEthernetStatsKey);
    if (!nd || !(fEthernetStats = (IOEthernetStats*)nd->getBuffer()))
        return false;
    return true;
}

IOReturn com_JMC_driver_MyDriver::getHardwareAddress(IOEthernetAddress *addrP)
{
    addrP->bytes[0] = 0xff;
    addrP->bytes[1] = 0xff;
    addrP->bytes[2] = 0xff;
    addrP->bytes[3] = 0xff;
    addrP->bytes[4] = 0xff;
    addrP->bytes[5] = 0xff;
    return kIOReturnSuccess;
}

IOReturn com_JMC_driver_MyDriver::enable(IONetworkInterface* netif)
{
    IOMediumType          mediumType = kIOMediumEthernet1000BaseT | kIOMediumOptionFullDuplex;
    IONetworkMedium*      medium;
    medium = IONetworkMedium::getMediumWithType(fMediumDict, mediumType);
    //if (!fHWAbstraction->enableHardware())
    if (0)
        return kIOReturnError;
    setLinkStatus(kIONetworkLinkActive | kIONetworkLinkValid, medium, 1000 * 1000000);
    return kIOReturnSuccess;
}

UInt32  com_JMC_driver_MyDriver::outputPacket(mbuf_t packet, void* param)
{
    IOReturn result = kIOReturnOutputSuccess;
    //if (fHWAbstraction->transmitPacketToHardware(packet) != kIOReturnSuccess)
    if (1)
    {
        result = kIOReturnOutputStall;
    }
    return result;
}

//-----------

static struct MediumTable
{
    UInt32      type;
    UInt32      speed;
}
mediumTable[] =
{
    {kIOMediumEthernetNone, 0},
    {kIOMediumEthernetAuto, 0},
    {kIOMediumEthernet10BaseT | kIOMediumOptionFullDuplex,  10},
    {kIOMediumEthernet100BaseTX | kIOMediumOptionFullDuplex, 100},
    {kIOMediumEthernet1000BaseT | kIOMediumOptionFullDuplex, 1000},
};
bool com_JMC_driver_MyDriver::createMediumDict()
{
    IONetworkMedium*  medium;
    UInt32                            i;
    
    fMediumDict = OSDictionary::withCapacity(sizeof(mediumTable) /
                                             sizeof(struct MediumTable));
    
    if (fMediumDict == 0)
        return false;
    
    for (i = 0; i < sizeof(mediumTable) / sizeof(struct MediumTable); i++)
    {
        medium = IONetworkMedium::medium(mediumTable[i].type, mediumTable[i].speed);
        if (medium)
        {
            IONetworkMedium::addMedium(fMediumDict, medium);
            medium->release();
        }
    }
    if (publishMediumDictionary(fMediumDict) != true)
        return false;
    medium = IONetworkMedium::getMediumWithType(fMediumDict, kIOMediumEthernetAuto);
    setSelectedMedium(medium);
    return true;
}