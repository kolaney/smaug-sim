# smaug-sim
This was made as a group project.  I unfortunately can't remember which contributions are mine and which were made by group members.

SIMULATION PARAMETERS (as taken from the original assignment description)

The first process in the simulation will spawn Smaug, and will spawn (over time) each of the cows, sheep, hunters and thieves.   

Sheep  arrive  (new  sheep  processes  are  created)  at  random  times.  The  length  of  the  interval between  the  arrivals  of  successive  sheep  is  drawn  from  a  uniform  distribution.  The  length  of interval n is the length of time between the arrival of sheep n and the arrival of sheep n+1.The distribution of possible interval lengths is uniform between 0 seconds and maximumSheepInterval seconds (0 probability of an arrival interval outside this range).  

Cows arrive (new cow processes are created) at random times. The length of the interval between the arrivals of successive cows is drawn from a uniform distribution. The distribution of possible interval lengths is uniform between 0 seconds and maximumCowInterval seconds (0 probability of an arrival interval outside this range).  

Similarly,  treasure  hunters  arrive  at  random  intervals  drawn  from  a  uniform  distribution.  The distribution of possible interval lengths is uniform between 0 seconds and maximumHunterInterval seconds (0 probability of an arrival interval outside this range). 

Similarly, thieves arrive at random intervals drawn from a uniform distribution. The distribution of possible  interval  lengths  is  uniform  between  0  seconds  and  maximumThiefInterval  or maximumHunterInterval  seconds  respectively  (0  probability  of  an  arrival  interval  outside  this range). 

Smaug’s treasure initially contains 400 jewels  

The probability that a particular treasure hunter or thief will not be defeated is winProb.  

Your  program  should  request  the  values  of  maximumSheepInterval,  maximumCowInterval, maximumHunterInterval, maximumThiefInterval and winProb from the user and use those values to initialize your simulation.  

The simulation will end when any of these conditions are met

Smaug has eaten 14 sheep and 14 cows 

Smaug has defeated 12 treasure hunters or 15 thieves 

Smaug has no treasure 

Smaug has 800 jewels. 