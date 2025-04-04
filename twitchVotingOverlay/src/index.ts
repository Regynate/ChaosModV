import { ChaosOverlayClient } from './chaosOverlayClient';
import { BarOverlay } from './barOverlay';

// Get DOM elements
const BAR_CONTAINER = document.getElementById('barContainer') as HTMLDivElement | null;
const TOTAL_VOTES = document.getElementById('totalVotes') as HTMLDivElement | null;
const CHANCE_ELEMENT = document.getElementById('chance') as HTMLDivElement | null;

if (BAR_CONTAINER === null) throw new Error('could not find bar container in DOM');
if (TOTAL_VOTES === null) throw new Error('could not find total votes element in DOM');
if (CHANCE_ELEMENT === null) throw new Error('could not find chance element in DOM');

const OVERLAY_CLIENT = new ChaosOverlayClient('ws://localhost:9091');

OVERLAY_CLIENT.addConnectListener(() => {
	TOTAL_VOTES.style.opacity = '1';
	CHANCE_ELEMENT.textContent = 'Chance';
});
OVERLAY_CLIENT.addDisconnectListener(() => {
	TOTAL_VOTES.style.opacity = '0';
	CHANCE_ELEMENT.textContent = 'Disconnected';
});
OVERLAY_CLIENT.addUpdateVoteListener(message => {
	TOTAL_VOTES.innerText = `Votes: ${message.totalVotes}`;
});

new BarOverlay(BAR_CONTAINER, OVERLAY_CLIENT);
