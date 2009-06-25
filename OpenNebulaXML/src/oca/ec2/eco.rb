
require 'rubygems'
require 'sinatra'
require 'EC2'

$: << './OpenNebulaApi'
$: << './lib'

require 'OpenNebula'
require 'repo_manager'

require 'pp'

include OpenNebula

ACCESS_KEY_ID = 'jfontan'
SECRET_ACCESS_KEY = 'opennebula'
SERVER = '127.0.0.1'
PORT = 4567

$repoman=RepoManager.new

def get_one_client
    Client.new('jfontan:opennebula')
end

def get_user(name)
    user=nil
    
    user_pool=UserPool.new(get_one_client)
    user_pool.info
    user_pool.each{|u|
        if u.name==name
            puts "yeah!"
            user=Hash.new
            user[:id]=u.id
            user[:name]=u.name
            user[:password]=u[:password]
        end
    }
    
    user
end

def authenticate(params)
    user_name=params['AWSAccessKeyId']
    user=get_user(user_name)
    
    return true
    
    halt 401, "User does not exist" if !user
    
    signature_params=params.reject {|key,value| key=='Signature' }
    canonical=EC2.canonical_string(signature_params, SERVER)
    signature=EC2.encode(user[:password], canonical, false)
    
    halt 401, "Bad password" if params['Signature']!=signature
end

before do
    authenticate(params)
end


def register_image(params)
    user=get_user(params['AWSAccessKeyId'])
    
    img=$repoman.add(user[:id], params["ImageLocation"])
    @img_id=img.uuid

    erb :register_image
end

def describe_images(params)
    @user=get_user(params['AWSAccessKeyId'])

    @images=Image.filter(:owner => @user[:id])
    
    pp @images
    
    erb :describe_images
end


def run_instances(params)
    @user=get_user(params['AWSAccessKeyId'])
    
    image_id=params['ImageId']
    image=$repoman.get(image_id)
    
    @vm_info=Hash.new
    @vm_info[:img_path]=image.path
    @vm_info[:img_id]=image_id
    
    template=ERB.new(File.read('templates/m1.small.erb'))
    template_text=template.result(binding)
    
    pp template_text
    
    vm=VirtualMachine.new(VirtualMachine.build_xml, get_one_client)
    response=vm.allocate(template_text)
    
    pp response
    
    @vm_info[:vm_id]=vm.id
    
    erb :run_instances
end

def describe_instances(params)
    @user=get_user(params['AWSAccessKeyId'])
    
    @vmpool=VirtualMachinePool.new(get_one_client)
    @vmpool.info
    
    erb :describe_instances
end

post '/' do
    pp params
    
    case params['Action']
    when 'RegisterImage'
        register_image(params)
    when 'DescribeImages'
        describe_images(params)
    when 'RunInstances'
        run_instances(params)
    when 'DescribeInstances'
        describe_instances(params)
    end
end


__END__

@@ register_image
<RegisterImageResponse xmlns="http://ec2.amazonaws.com/doc/2009-04-04/"> 
  <imageId><%= @img_id %></imageId> 
</RegisterImageResponse>


@@ describe_images
<DescribeImagesResponse xmlns="http://ec2.amazonaws.com/doc/2009-04-04/"> 
  <imagesSet> 
  <% for image in @images %>
    <item> 
      <imageId><%= image.uuid %></imageId> 
      <imageLocation><%= image.path %></imageLocation> 
      <imageState>available</imageState> 
      <imageOwnerId><%= @user[:name] %></imageOwnerId> 
      <isPublic>false</isPublic> 
      <architecture>i386</architecture> 
      <imageType>machine</imageType> 
    </item> 
  <% end %>
  </imagesSet> 
</DescribeImagesResponse> 


@@ run_instances
<RunInstancesResponse xmlns="http://ec2.amazonaws.com/doc/2009-04-04/"> 
  <reservationId>r-47a5402e</reservationId> 
  <ownerId><%= @user[:name] %></ownerId> 
  <groupSet> 
    <item> 
      <groupId>default</groupId> 
    </item> 
  </groupSet> 
  <instancesSet> 
    <item> 
      <instanceId><%= @vm_info[:vm_id] %></instanceId> 
      <imageId><%= @vm_info[:img_id] %></imageId> 
      <instanceState> 
        <code>0</code> 
        <name>pending</name> 
      </instanceState> 
      <privateDnsName></privateDnsName> 
      <dnsName></dnsName> 
      <keyName>example-key-name</keyName> 
      <amiLaunchIndex>0</amiLaunchIndex> 
      <instanceType>m1.small</instanceType> 
      <launchTime>2007-08-07T11:51:50.000Z</launchTime> 
      <placement> 
        <availabilityZone>us-east-1b</availabilityZone> 
      </placement> 
      <monitoring> 
        <enabled>true</enabled> 
      </monitoring> 
    </item> 
  </instancesSet> 
</RunInstancesResponse> 

@@ describe_instances
<DescribeInstancesResponse xmlns="http://ec2.amazonaws.com/doc/2009-04-04/"> 
  <reservationSet> 
    <item> 
      <reservationId>default</reservationId> 
      <ownerId><%= @user[:name] %></ownerId> 
      <groupSet> 
        <item> 
          <groupId>default</groupId> 
        </item> 
      </groupSet> 
      <instancesSet> 
        <% @vmpool.each do |vm| %>
        <item> 
          <instanceId><%= vm.id %></instanceId> 
          <imageId><%= vm.id %></imageId> 
          <instanceState> 
            <code>0</code> 
            <name>running</name> 
          </instanceState> 
          <privateDnsName>10-251-50-132.ec2.internal</privateDnsName> 
          <dnsName>ec2-72-44-33-4.compute-1.amazonaws.com</dnsName> 
          <keyName>example-key-name</keyName> 
          <amiLaunchIndex>23</amiLaunchIndex> 
          <productCodesSet> 
            <item><productCode>774F4FF8</productCode></item> 
          </productCodesSet> 
          <instanceType>m1.large</instanceType> 
          <launchTime>2007-08-07T11:54:42.000Z</launchTime> 
          <placement> 
            <availabilityZone>us-east-1b</availabilityZone> 
          </placement> 
        </item> 
        <% end %>
      </instancesSet>
    </item>
  </reservationSet>
</DescribeInstancesResponse>
  

