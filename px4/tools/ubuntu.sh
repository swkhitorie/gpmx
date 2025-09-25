
INSTALL_ARCH=`uname -m`

echo "[ubuntu.sh] Starting..."
echo "[ubuntu.sh] arch: $INSTALL_ARCH"

# detect if running in docker
if [ "$RUNS_IN_DOCKER" = "true" ]; then
    echo "[ubuntu.sh] FAILED: no support in docker"
	exit 1
fi

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# check requirements.txt exists (script not run in source tree)
REQUIREMENTS_FILE="requirements.txt"
if [[ ! -f "${DIR}/${REQUIREMENTS_FILE}" ]]; then
	echo "[ubuntu.sh] FAILED: ${REQUIREMENTS_FILE} needed in same directory as ubuntu.sh (${DIR})."
	exit 1
fi

# Linux Mint compatibility: use upstream Ubuntu values
if [ -r /etc/upstream-release/lsb-release ]; then
    . /etc/upstream-release/lsb-release
    UBUNTU_CODENAME="${DISTRIB_CODENAME:-${UBUNTU_CODENAME:-}}"
    UBUNTU_RELEASE="${DISTRIB_RELEASE:-${UBUNTU_RELEASE:-}}"

    lsb_release() {
        if [ "$1" = "-cs" ]; then
            printf '%s' "$UBUNTU_CODENAME"
        elif [ "$1" = "-rs" ]; then
            printf '%s' "$UBUNTU_RELEASE"
        else
            command lsb_release "$@"
        fi
    }
fi

# check ubuntu version
# otherwise warn and point to docker?
UBUNTU_RELEASE="`lsb_release -rs`"
echo "[ubuntu.sh] Ubuntu ${UBUNTU_RELEASE}"
echo "[ubuntu.sh] Installing PX4 general dependencies"

sudo apt-get update -y --quiet
sudo DEBIAN_FRONTEND=noninteractive apt-get -y --quiet --no-install-recommends install \
	astyle \
	build-essential \
	ccache \
	cmake \
	cppcheck \
	file \
	g++ \
	gcc \
	gdb \
	git \
	lcov \
	libssl-dev \
	libxml2-dev \
	libxml2-utils \
	make \
	ninja-build \
	python3 \
	python3-dev \
	python3-pip \
	python3-setuptools \
	python3-wheel \
	rsync \
	shellcheck \
	unzip \
	zip \
	;

# Python3 dependencies
echo
echo "[ubuntu.sh] Installing PX4 Python3 dependencies"
PYTHON_VERSION=$(python3 --version 2>&1 | awk '{print $2}')
REQUIRED_VERSION="3.11"
if [[ "$(printf '%s\n' "$REQUIRED_VERSION" "$PYTHON_VERSION" | sort -V | head -n1)" == "$REQUIRED_VERSION" ]]; then
	python3 -m pip install --break-system-packages -r ${DIR}/requirements.txt
else
	if [ -n "$VIRTUAL_ENV" ]; then
		# virtual environments don't allow --user option
		python -m pip install -r ${DIR}/requirements.txt
	else
		python3 -m pip install --user -r ${DIR}/requirements.txt
	fi
fi

