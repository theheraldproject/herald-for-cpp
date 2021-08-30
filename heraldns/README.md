# Herald Network Simulations

This folder contains high performance code for network effects simulation.

## Aims

The aims of this investigate spike is to test hypotheses around advances
to social mixing score calculations that take in to account network effects
rather than point to point risk observations.

The ideal outcome is to come up with some consistent measure or heuristic
that would indicate a persons risk exposure, or speed of adding to risk
exposure, whilst performing normal day to day activities.

Digital Contact Tracing (DCT) aims to trace people whose risk exposure
has been increased from known, or suspected, cases of disease exposure.
This requires some sort of centralised system, either for exposure key 
exchange only (decentralised model) or for more advanced exposure network
analysis (centralised and hybrid models).

Fundamentally DCT is about alerting people who have already been exposed
at a risk high enough for them to be likely to have contracted a disease.

This spike is instead trying to discover a method of phone-to-phone
risk exposure calculation that provides individuals with information
to make their own personal choices around social mixing PRIOR to being
placed at risk of enough exposure to contract a disease.

This method still requires network effects, but ideally will use an approach
that does not 'out' people as serious sources of potential infection,
but gives a reliable approximation to risk over an unknown network of nodes.

If this can be achieved it will have the following benefits:-
- Helps people prevent their exposure, or track their current risk, prior to reaching a threshold where they are likely to have already contracted a disease
   - And thus also prevent onward exposure before the asymptomatic period is over, or before a DCT app notifies an individual of exposure
- Allow a single app to be published for all communities worldwide without causing privacy, data protection, or security concerns
- Give societies and individuals an early indication of likely disease spread to assist with modelling or outbreak prevention

## Hypotheses

Below are high level hypotheses that require testing

### H1. Direct exposure and social mixing score

Direct individual to individual (first order) pre-emptive risk scoring (social mixing score) 
gives a moderately accurate analogue to exposure notification based on confirmed case testing (DCT).

Settings:-
- Random start positions - try 1000x1000 grid with 1000 people
- Start with 40 cases at Day 0 of COVID-19, assume no isolation or testing (simple scenario), with the person not being a spreader after 14 days, and not able to contract for another 60 days
- Actual state of person moves to 'contracted' if any single event, or mix of events, passes '15 minutes 2 metres', with 100% contraction rate (Basic Transmission model)
- Assume 100% distance estimation accuracy (simple distance calculation)
- Run the simulation for 200 days
- Assume each tick is 5 minutes
- Assume each square is 0.5 metres
- Use Oxford Risk Model for actual transmission risk calculation
- Use distance (not RSSI yet) multipled by minutes for the risk formula
   - Assumes all phones to have identical radio characteristics and perfect distance conversion

Expected results:-
- High degree of transmission over time, showing a peak and fall of number of cases
- Strong correlation between individuals with 'peak RSSI-minutes' and 'did fall ill'

### H2. Network effect exposure and social mixing score

A social mixing score based on your contacts, and their recent contacts, for a known
contact graph gives a more accurate analogus to exposure notification based on confirmed
case testing (DCT).

### H3. Distributed decentralised risk scoring

A method of distributing social mixing scoring can be derived to ensure privacy of individuals,
and of their contacts, whilst still providing an accurate analogue to DCT exposure notification.

### H4. Personal action

If individuals using the app are provided with this information, and decide to take
action to limit their mixing, this will lower the transmission of a disease.

### H5. Low uptake

This method of risk scoring can still provide a useful control effect with low uptake of the
app, and moderate self control based on social mixing score.

## General test method

We shall create a simulation such that:-

- There is a grid for a large populated area. Each grid squares size can be varied. Each grid square can contain multiple people
- Distance of number of squares relates to exposure distance
- For each time period, tN, a person may move or may stay stationary
- Each person starts with a model risk score number
- Over time as more people are closer to each other, the particular model being tested with calculate new accrued risk per time period 'tick'
- Separately, a social mixing score calculation method will also be executed (allowing comparison between 'actual' and 'scored' risk)
- At the end of each tick, the new risk score will be applied, and the next time tick is executed
- The above process repeats for an entire simulation period

The variables for the simulation will vary according to what is being tested.

The output data will be saved as CSV files (settings.csv, results.csv, cases.csv, people.csv). Multiple simulations of the same settings shall be ran
in order to ensure repeatability of the test.
